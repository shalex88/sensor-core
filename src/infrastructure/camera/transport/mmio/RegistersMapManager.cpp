#include "RegistersMapManager.h"

#include <ranges>

#include "RegistersMap.h"
#include "infrastructure/camera/transport/mmio/IRegisterImpl.h"

namespace service::infrastructure {
    RegistersMapManager::RegistersMapManager(std::unique_ptr<IRegisterImpl> impl) : register_(std::move(impl)) {
    }

    RegistersMapManager::~RegistersMapManager() = default;

    Result<uint32_t> RegistersMapManager::getValue(const REG reg) const {
        std::lock_guard lock(mutex_);
        const auto result = register_->get(g_registers_map[reg].address);
        if (result.isError()) {
            return Result<uint32_t>::error("Failed to read register value: " + result.error());
        }
        return Result<uint32_t>::success(result.value());
    }

    Result<void> RegistersMapManager::setValue(const REG reg, const uint32_t value) const {
        std::lock_guard lock(mutex_);
        if (const auto result = register_->set(g_registers_map[reg].address, value); result.isError()) {
            return Result<void>::error("Failed to write register value: " + result.error());
        }
        return Result<void>::success();
    }

    Result<void> RegistersMapManager::resetValue(const REG reg) const {
        return setValue(reg, g_registers_map[reg].default_value);
    }

    Result<void> RegistersMapManager::clearValue(const REG reg) const {
        return setValue(reg, 0);
    }

    Result<void> RegistersMapManager::setBit(const REG reg, const uint8_t bit_index) const {
        if (bit_index > 31) {
            return Result<void>::error("Bit index out of range (0-31)");
        }

        const auto reg_result = getValue(reg);
        if (reg_result.isError()) {
            return Result<void>::error("Failed to read register for bit set: " + reg_result.error());
        }

        auto reg_value = reg_result.value();
        reg_value |= 1UL << bit_index;

        return setValue(reg, reg_value);
    }

    Result<void> RegistersMapManager::clearBit(const REG reg, const uint8_t bit_index) const {
        if (bit_index > 31) {
            return Result<void>::error("Bit index out of range (0-31)");
        }

        const auto reg_result = getValue(reg);
        if (reg_result.isError()) {
            return Result<void>::error("Failed to read register for bit clear: " + reg_result.error());
        }

        auto reg_value = reg_result.value();
        reg_value &= ~(1UL << bit_index);

        return setValue(reg, reg_value);
    }

    Result<uint8_t> RegistersMapManager::getNibble(const REG reg, const uint8_t nibble_index) const {
        if (nibble_index > 7) {
            return Result<uint8_t>::error("Nibble index out of range (0-7)");
        }

        const auto reg_result = getValue(reg);
        if (reg_result.isError()) {
            return Result<uint8_t>::error("Failed to read register for nibble get: " + reg_result.error());
        }

        const auto reg_value = reg_result.value();
        const uint8_t nibble_value = (reg_value >> (nibble_index * 4)) & 0xF;

        return Result<uint8_t>::success(nibble_value);
    }

    Result<void> RegistersMapManager::setNibble(const REG reg, const uint8_t nibble_index,
                                                const uint8_t nibble_value) const {
        if (nibble_index > 7) {
            return Result<void>::error("Nibble index out of range (0-7)");
        }

        if (nibble_value > 0xF) {
            return Result<void>::error("Nibble value out of range (0-15)");
        }

        const auto reg_result = getValue(reg);
        if (reg_result.isError()) {
            return Result<void>::error("Failed to read register for nibble set: " + reg_result.error());
        }

        auto reg_value = reg_result.value();
        reg_value = (reg_value & ~(0xF << (nibble_index * 4))) | (nibble_value << (nibble_index * 4));

        return setValue(reg, reg_value);
    }

    Result<void> RegistersMapManager::resetAll() const {
        for (const auto key : g_registers_map | std::views::keys) {
            if (const auto result = setValue(key, g_registers_map[key].default_value); result.isError()) {
                return Result<void>::error(std::string("Failed to reset register: ") + result.error());
            }
        }
        return Result<void>::success();
    }

    Result<void> RegistersMapManager::clearAll() const {
        for (const auto key : g_registers_map | std::views::keys) {
            if (const auto result = setValue(key, 0); result.isError()) {
                return Result<void>::error(std::string("Failed to clear register: ") + result.error());
            }
        }
        return Result<void>::success();
    }
} // namespace service::infrastructure
