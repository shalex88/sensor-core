#pragma once

#include <string>

#include "IRegisterImpl.h"

namespace service::infrastructure {
    class RegisterImplUio final : public IRegisterImpl {
    public:
        explicit RegisterImplUio(std::string device);
        ~RegisterImplUio() override;

        // Delete copy constructor and assignment operator
        RegisterImplUio(const RegisterImplUio&) = delete;
        RegisterImplUio& operator=(const RegisterImplUio&) = delete;

        // Allow move constructor and assignment operator
        RegisterImplUio(RegisterImplUio&& other) noexcept;
        RegisterImplUio& operator=(RegisterImplUio&& other) noexcept;

        Result<void> set(uint32_t address, uint32_t value) override;
        Result<uint32_t> get(uint32_t address) const override;

        bool isOpen() const noexcept {
            return fd_ != -1 && mapped_memory_ != nullptr;
        }

    private:
        bool open();
        void close();
        bool mapMemory();
        void unmapMemory();
        bool getUioInfo();
        std::string getUioNameFromDevicePath() const;
        static uint64_t getUioBaseAddress(const std::string& uio_name);
        static size_t getUioSize(const std::string& uio_name);
        bool isValidAddress(uint32_t address) const;
        uint64_t calculateOffset(uint32_t address) const;

        std::string device_;
        int fd_{-1};
        void* mapped_memory_{nullptr};
        size_t memory_size_{0};
        uint64_t base_address_{0};
    };
} // namespace service::infrastructure
