#include "RegisterImplUio.h"

#include <fcntl.h>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <unistd.h>
#include <utility>
#include <sys/mman.h>
#include <sys/stat.h>

namespace service::infrastructure {
    RegisterImplUio::RegisterImplUio(std::string device) : device_(std::move(device)) {
        if (!open()) {
            throw std::runtime_error("Failed to open UIO device: " + device_);
        }

        if (!getUioInfo()) {
            close();
            throw std::runtime_error("Failed to read UIO device information from sysfs for: " + device_);
        }

        if (!mapMemory()) {
            close();
            throw std::runtime_error("Failed to map UIO device memory for: " + device_);
        }
    }

    RegisterImplUio::~RegisterImplUio() {
        close();
    }

    RegisterImplUio::RegisterImplUio(RegisterImplUio&& other) noexcept : device_(std::move(other.device_)),
                                                                         fd_(other.fd_),
                                                                         mapped_memory_(other.mapped_memory_),
                                                                         memory_size_(other.memory_size_),
                                                                         base_address_(other.base_address_) {
        other.fd_ = -1;
        other.mapped_memory_ = nullptr;
        other.memory_size_ = 0;
        other.base_address_ = 0;
    }

    RegisterImplUio& RegisterImplUio::operator=(RegisterImplUio&& other) noexcept {
        if (this != &other) {
            close();

            device_ = std::move(other.device_);
            fd_ = other.fd_;
            mapped_memory_ = other.mapped_memory_;
            memory_size_ = other.memory_size_;
            base_address_ = other.base_address_;

            other.fd_ = -1;
            other.mapped_memory_ = nullptr;
            other.memory_size_ = 0;
            other.base_address_ = 0;
        }
        return *this;
    }

    Result<void> RegisterImplUio::set(const uint32_t address, const uint32_t value) {
        if (!isValidAddress(address)) {
            return Result<void>::error("Invalid address: 0x" + std::to_string(address));
        }

        const uint64_t offset = calculateOffset(address);

        // Cast to volatile to prevent compiler optimizations
        volatile auto* reg_ptr = static_cast<volatile uint32_t*>(static_cast<volatile void*>(static_cast<char*>(
            mapped_memory_) + offset));
        *reg_ptr = value;
        return Result<void>::success();
    }

    Result<uint32_t> RegisterImplUio::get(const uint32_t address) const {
        if (!isValidAddress(address)) {
            return Result<uint32_t>::error("Invalid address: 0x" + std::to_string(address));
        }

        const uint64_t offset = calculateOffset(address);

        // Cast to volatile to prevent compiler optimizations
        const volatile auto* reg_ptr = static_cast<const volatile uint32_t*>(static_cast<const volatile void*>(
            static_cast<const char*>(mapped_memory_) + offset));
        const uint32_t value = *reg_ptr;
        return Result<uint32_t>::success(value);
    }

    bool RegisterImplUio::open() {
        fd_ = ::open(device_.c_str(), O_RDWR | O_SYNC);
        return fd_ != -1;
    }

    void RegisterImplUio::close() {
        unmapMemory();

        if (fd_ != -1) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    bool RegisterImplUio::mapMemory() {
        if (fd_ == -1 || memory_size_ == 0) {
            return false;
        }

        mapped_memory_ = mmap(nullptr, memory_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mapped_memory_ == MAP_FAILED) {
            mapped_memory_ = nullptr;
            return false;
        }

        return true;
    }

    void RegisterImplUio::unmapMemory() {
        if (mapped_memory_ != nullptr && mapped_memory_ != MAP_FAILED) {
            munmap(mapped_memory_, memory_size_);
            mapped_memory_ = nullptr;
        }
        memory_size_ = 0;
    }

    bool RegisterImplUio::getUioInfo() {
        const std::string uio_name = getUioNameFromDevicePath();
        if (uio_name.empty()) {
            return false;
        }

        base_address_ = getUioBaseAddress(uio_name);
        if (base_address_ == 0) {
            return false;
        }

        memory_size_ = getUioSize(uio_name);
        if (memory_size_ == 0) {
            return false;
        }

        return true;
    }

    std::string RegisterImplUio::getUioNameFromDevicePath() const {
        // Extract UIO number from device path (e.g., "/dev/uio0" -> "uio0")
        const std::regex uio_regex(R"(/dev/(uio\d+))");

        if (std::smatch match; std::regex_search(device_, match, uio_regex)) {
            return match[1].str();
        }

        return "";
    }

    uint64_t RegisterImplUio::getUioBaseAddress(const std::string& uio_name) {
        const std::string addr_file = "/sys/class/uio/" + uio_name + "/maps/map0/addr";
        std::ifstream file(addr_file);

        if (!file.is_open()) {
            return 0;
        }

        std::string addr_str;
        std::getline(file, addr_str);

        if (addr_str.empty()) {
            return 0;
        }

        return std::stoull(addr_str, nullptr, 16);
    }

    size_t RegisterImplUio::getUioSize(const std::string& uio_name) {
        const std::string size_file = "/sys/class/uio/" + uio_name + "/maps/map0/size";
        std::ifstream file(size_file);

        if (!file.is_open()) {
            return 0;
        }

        std::string size_str;
        std::getline(file, size_str);

        if (size_str.empty()) {
            return 0;
        }
        return std::stoull(size_str, nullptr, 16);
    }

    bool RegisterImplUio::isValidAddress(const uint32_t address) const {
        if (!isOpen()) {
            return false;
        }

        // Check if address is below the base address
        if (static_cast<uint64_t>(address) < base_address_) {
            return false;
        }

        // Calculate offset to check bounds
        const uint64_t offset = calculateOffset(address);

        // Check if offset is within mapped memory bounds
        if (offset + sizeof(uint32_t) > memory_size_) {
            return false;
        }

        return true;
    }

    uint64_t RegisterImplUio::calculateOffset(const uint32_t address) const {
        return static_cast<uint64_t>(address) - base_address_;
    }
} // namespace service::infrastructure
