#include "FpgaTransport.h"

#include <fcntl.h>
#include <fstream>
#include <regex>
#include <utility>
#include <sys/mman.h>

#include "common/logger/Logger.h"

namespace service::infrastructure {
    namespace {
        constexpr uint32_t FPGA_BASE_ADDR = 0x90000000;
        constexpr uint32_t FPGA_MEMORY_SIZE = 0x4000000;

        enum class FpgaRegs : uint32_t {
            StartStop = 0x00,
            ReadWrite = 0x04,
            ReadLastData = 0x08,
            WriteData = 0x0C,
            WriteCounter = 0x10,
            NumOfWrites = 0x14,
        };

        enum class HostReg : uint32_t {
            SelectChannel = 0x0,
            WorkingSpeed = 0x4,
            LinkStatus = 0x8,
            Reset = 0x2000,
            StreamId = 0x2018,
            CameraIndex = 0x40,
            CameraArbitration = 0x3C,
            HostDecoder = 0x2034
        };
    } // unnamed namespace

    FpgaTransport::FpgaTransport(std::string device)
        : device_(std::move(device)), base_address_(FPGA_BASE_ADDR), memory_size_(FPGA_MEMORY_SIZE)  {
        if (!open() || !mapMemory()) {
            throw std::runtime_error("Failed to open device: " + device_);
        }

        if (!configureLink()) {
            close();
            throw std::runtime_error("Failed to connect to a camera: " + device_);
        }
    }

    FpgaTransport::~FpgaTransport() {
        close();
    }

    void FpgaTransport::Read(void* buffer, const int64_t address, const int64_t length) {
        const uint32_t val = readReg(Target::Camera, static_cast<uint32_t>(address));
        std::memcpy(buffer, &val, std::min<int64_t>(length, 4));
    }

    void FpgaTransport::Write(const void* buffer, const int64_t address, const int64_t length) {
        uint32_t val = 0;
        std::memcpy(&val, buffer, std::min<int64_t>(length, 4));
        writeReg(Target::Camera, static_cast<uint32_t>(address), val);
    }

    GENAPI_NAMESPACE::EAccessMode FpgaTransport::GetAccessMode() const {
        return GENAPI_NAMESPACE::RW;
    }

    bool FpgaTransport::open() {
        fd_ = ::open(device_.c_str(), O_RDWR | O_SYNC);
        return fd_ != -1;
    }

    bool FpgaTransport::mapMemory() {
        if (fd_ == -1 || memory_size_ == 0) {
            return false;
        }

        mapped_memory_ = static_cast<volatile uint32_t*>(mmap(nullptr, memory_size_, PROT_READ | PROT_WRITE, MAP_SHARED,
                                                              fd_, base_address_));
        if (mapped_memory_ == MAP_FAILED) {
            return false;
        }

        return true;
    }

    void FpgaTransport::unmapMemory() {
        if (mapped_memory_ != MAP_FAILED) {
            munmap(const_cast<uint32_t*>(mapped_memory_), memory_size_);
            mapped_memory_ = nullptr;
        }
        memory_size_ = 0;
    }

    void FpgaTransport::close() {
        unmapMemory();

        if (fd_ != -1) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    bool FpgaTransport::configureLink() const {
        // [CXP] Channel select 0
        writeReg(Target::Host, HostReg::SelectChannel, 0x0);
        readReg(Target::Host, HostReg::SelectChannel);

        // [CXP] Link speed discovery 3.125 Gbps
        writeReg(Target::Host, HostReg::WorkingSpeed, 0x38);
        readReg(Target::Host, HostReg::WorkingSpeed);

        // [CXP] Reset link on camera side
        writeReg(Target::Camera, 0x4000, 0x1);
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        // [CXP] Read link status (bit0=1 expected)
        if (readReg(Target::Host, HostReg::LinkStatus) != 0x1) {
            return false;
        }

        // [CXP] Read magic number (expect 0xC0A79AE5)
        readReg(Target::Camera, 0x0);

        // [CXP] Set link speed to camera 3.125 Gbps
        writeReg(Target::Camera, 0x4014, 0x38);
        readReg(Target::Camera, 0x4014);

        // [CXP] Confirm link speed discovery again
        writeReg(Target::Host, HostReg::WorkingSpeed, 0x38);
        readReg(Target::Host, HostReg::WorkingSpeed);

        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        // [CXP] Read link status (bits0:1 = 11 expected)
        readReg(Target::Host, HostReg::LinkStatus);

        // [CXP] Read magic again
        readReg(Target::Camera, 0x0);

        // [CXP] Write MasterHostConnectionID = 0xDE000000
        writeReg(Target::Camera, 0x4008, 0xDE000000);
        readReg(Target::Camera, 0x4008);

        // [CXP] Packet size = 2048
        writeReg(Target::Camera, 0x4010, 0x800);
        readReg(Target::Camera, 0x4010);

        // [CXP] Decoder reset
        writeReg(Target::Host, HostReg::Reset, 0x2);
        readReg(Target::Host, HostReg::Reset);

        // [CXP] Stream id[15..8]=1, channel mask[7..0]=1
        writeReg(Target::Host, HostReg::StreamId, 0x00000100);
        readReg(Target::Host, HostReg::StreamId);

        // [CXP] Select camera 0
        writeReg(Target::Host, HostReg::CameraIndex, 0x0);
        readReg(Target::Host, HostReg::CameraIndex);

        // [CXP] Connect link0->arbiter0
        writeReg(Target::Host, HostReg::CameraArbitration, 0x1);
        readReg(Target::Host, HostReg::CameraArbitration);

        // [CXP] Connect arbiter0->decoder0
        writeReg(Target::Host, HostReg::HostDecoder, 0x0);
        readReg(Target::Host, HostReg::HostDecoder);

        // [CXP] Decoder enable
        writeReg(Target::Host, HostReg::Reset, 0x1);
        readReg(Target::Host, HostReg::Reset);

        return true; //TODO: add failure checks
    }

    void FpgaTransport::writeTransaction(const uint32_t target_reg, const uint32_t value) const {
        auto idx = [](FpgaRegs r) { return static_cast<uint32_t>(r) >> 2; };
        mapped_memory_[idx(FpgaRegs::StartStop)] = 0;
        mapped_memory_[idx(FpgaRegs::ReadWrite)] = 1;
        mapped_memory_[idx(FpgaRegs::NumOfWrites)] = 2;
        mapped_memory_[idx(FpgaRegs::WriteData)] = target_reg;
        mapped_memory_[idx(FpgaRegs::WriteCounter)] = 0;
        mapped_memory_[idx(FpgaRegs::WriteData)] = value;
        mapped_memory_[idx(FpgaRegs::WriteCounter)] = 1;
        mapped_memory_[idx(FpgaRegs::StartStop)] = 1;
    }

    uint32_t FpgaTransport::readTransaction(const uint32_t target_reg) const {
        auto idx = [](FpgaRegs r) { return static_cast<uint32_t>(r) >> 2; };
        mapped_memory_[idx(FpgaRegs::StartStop)] = 0;
        mapped_memory_[idx(FpgaRegs::ReadWrite)] = 0;
        mapped_memory_[idx(FpgaRegs::WriteData)] = target_reg;
        mapped_memory_[idx(FpgaRegs::WriteCounter)] = 0;
        mapped_memory_[idx(FpgaRegs::StartStop)] = 1;

        return mapped_memory_[idx(FpgaRegs::ReadLastData)];
    }

    void FpgaTransport::writeReg(const Target target, const auto reg, const uint32_t value) const {
        //TODO: allow reg to be only enum
        const auto target_reg = static_cast<uint32_t>(target) + static_cast<uint32_t>(reg);
        writeTransaction(target_reg, value);
        LOG_DEBUG("[0x{:X}] <- 0x{:X}", target_reg, value);
    }

    uint32_t FpgaTransport::readReg(const Target target, const auto reg) const {
        //TODO: allow reg to be only enum
        const auto target_reg = static_cast<uint32_t>(target) + static_cast<uint32_t>(reg);
        const uint32_t value = readTransaction(target_reg);
        LOG_DEBUG("[0x{:X}] -> 0x{:X}", target_reg, value);

        return value;
    }
}