#pragma once

#include <array>
#include <cstddef>

namespace service::infrastructure {
    // MWIR Camera opcodes - using std::byte arrays for type safety
    constexpr std::array<std::byte, 4> MWIR_GET_VERSION{
        std::byte{0x00}, std::byte{0x20}, std::byte{0x00}, std::byte{0x00}
    };
} // namespace service::infrastructure