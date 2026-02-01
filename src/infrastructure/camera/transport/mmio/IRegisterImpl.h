#pragma once

#include <cstdint>
#include "common/types/Result.h"

namespace service::infrastructure {
    class IRegisterImpl {
    public:
        virtual ~IRegisterImpl() = default;
        virtual Result<void> set(uint32_t address, uint32_t value) = 0;
        virtual Result<uint32_t> get(uint32_t address) const = 0;
    };
} // namespace service::infrastructure