#pragma once

#include <cstddef>
#include <span>
#include <string_view>

#include "common/types/Result.h"

namespace service::infrastructure {
    class ITransport {
    public:
        virtual ~ITransport() = default;
        virtual Result<void> open() = 0; //TODO: do we need it in the interface or only use RAII?
        virtual Result<void> close() = 0; //TODO: do we need it in the interface or only use RAII?
        virtual Result<void> write(std::span<const std::byte> frame) = 0;
        virtual Result<size_t> read(std::span<std::byte> buffer) = 0;
        virtual bool isOpen() const = 0;

        Result<void> write(std::string_view sv) {
            const std::span chars{sv.data(), sv.size()};
            const auto bytes = std::as_bytes(chars);
            return write(bytes);
        }

        Result<void> write(const std::span<const uint8_t> frame) {
            const std::span bytes{reinterpret_cast<const std::byte*>(frame.data()), frame.size()};
            return write(bytes);
        }

        Result<size_t> read(std::span<uint8_t> buffer) {
            const std::span bytes{reinterpret_cast<std::byte*>(buffer.data()), buffer.size()};
            return read(bytes);
        }
    };
} // namespace service::infrastructure