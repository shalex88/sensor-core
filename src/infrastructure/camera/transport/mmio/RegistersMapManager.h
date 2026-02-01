#pragma once

#include <memory>
#include <mutex>

#include "RegistersMap.h"
#include "common/types/Result.h"

namespace service::infrastructure {
    class IRegisterImpl;

    class RegistersMapManager {
    public:
        explicit RegistersMapManager(std::unique_ptr<IRegisterImpl> impl);
        ~RegistersMapManager();

        Result<void> setValue(REG reg, uint32_t value) const;
        Result<uint32_t> getValue(REG reg) const;
        Result<void> resetValue(REG reg) const;
        Result<void> clearValue(REG reg) const;
        Result<void> setBit(REG reg, uint8_t bit_index) const;
        Result<void> clearBit(REG reg, uint8_t bit_index) const;
        Result<uint8_t> getNibble(REG reg, uint8_t nibble_index) const;
        Result<void> setNibble(REG reg, uint8_t nibble_index, uint8_t nibble_value) const;
        Result<void> resetAll() const;
        Result<void> clearAll() const;

    private:
        std::unique_ptr<IRegisterImpl> register_;
        mutable std::mutex mutex_;
    };
} // namespace service::infrastructure
