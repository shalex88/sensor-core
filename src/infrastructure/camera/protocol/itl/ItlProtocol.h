#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <span>
#include <vector>

#include "common/types/Result.h"

namespace service::infrastructure {
    class ITransport;

    struct __attribute__((packed)) ItlHeader {
        std::array<std::byte, 4> opcode{};
        std::array<std::byte, 4> id{std::byte{'F'}, std::byte{'R'}, std::byte{'T'}, std::byte{'R'}};
        std::array<std::byte, 2> length{};
        std::array<std::byte, 2> counter{};
        std::array<std::byte, 4> time_stamp{};
        std::byte source{};
        std::byte destination{};
        std::array<std::byte, 2> checksum{};
    };

    struct ItlMessage {
        ItlHeader header;
        std::vector<std::byte> payload;
    };

    class ItlProtocol {
    public:
        explicit ItlProtocol(std::unique_ptr<ITransport> transport);
        ~ItlProtocol();

        Result<void> open() const;
        Result<void> close() const;
        Result<std::vector<std::byte>> sendPayload(std::array<std::byte, 4> opcode, std::span<const std::byte> payload) const;

    private:
        std::unique_ptr<ITransport> transport_;
        mutable std::array<std::byte, 1024> rx_buffer_{};
        static std::vector<std::byte> createMessage(std::array<std::byte, 4> opcode, std::span<const std::byte> payload);
        static std::vector<std::byte> serialize(const ItlMessage& message);
        static std::vector<std::byte> serializeHeader(const ItlHeader& header);
        static Result<ItlMessage> deserialize(std::span<const std::byte> data);
        static void printMessage(std::span<const std::byte> message);
        static std::array<std::byte, 2> calculateXorChecksum(std::span<const std::byte> data);
        static std::array<std::byte, 2> calculateMessageChecksum(const ItlMessage& message);
        static bool isValidChecksum(const ItlMessage& message);

        // Helper functions for converting between multibyte values and byte arrays
        static std::array<std::byte, 2> toBytes(uint16_t value);
        static uint16_t fromBytes(std::span<const std::byte, 2> bytes);
    };
} // namespace service::infrastructure