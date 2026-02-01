#include "ItlProtocol.h"

#include <string>

#include "infrastructure/camera/transport/ITransport.h"

namespace service::infrastructure {
    ItlProtocol::ItlProtocol(std::unique_ptr<ITransport> transport)
        : transport_(std::move(transport)) {
    }

    ItlProtocol::~ItlProtocol() = default;

    Result<void> ItlProtocol::open() const {
        return transport_->open();
    }

    Result<void> ItlProtocol::close() const {
        return transport_->close();
    }

    Result<std::vector<std::byte>> ItlProtocol::sendPayload(std::array<std::byte, 4> opcode, std::span<const std::byte> payload) const {
        const auto message = createMessage(opcode, payload);
        if (const auto send_result = transport_->write(message); send_result.isError()) {
            return Result<std::vector<std::byte>>::error(send_result.error());
        }

        if (const auto serialized_response = transport_->read(rx_buffer_); serialized_response.isError()) {
            return Result<std::vector<std::byte>>::error(serialized_response.error());
        }

        const auto deserialized_response = deserialize(rx_buffer_);
        if (deserialized_response.isError()) {
            return Result<std::vector<std::byte>>::error(deserialized_response.error());
        }

        return Result<std::vector<std::byte>>::success(deserialized_response.value().payload);
    }

    std::vector<std::byte> ItlProtocol::createMessage(std::array<std::byte, 4> opcode, std::span<const std::byte> payload) {
        ItlMessage message;
        message.payload.assign(payload.begin(), payload.end());
        message.header.opcode = opcode;

        const uint16_t total_length = sizeof(message.header) + payload.size();
        message.header.length = toBytes(static_cast<uint16_t>(total_length));
        message.header.checksum = calculateMessageChecksum(message);

        auto serialized = serialize(message);
        printMessage(serialized);

        return serialized;
    }

    std::vector<std::byte> ItlProtocol::serialize(const ItlMessage& message) {
        auto serialized_message = serializeHeader(message.header);
        serialized_message.insert(serialized_message.end(), message.payload.begin(), message.payload.end());

        return serialized_message;
    }

    std::vector<std::byte> ItlProtocol::serializeHeader(const ItlHeader& header) {
        std::vector<std::byte> serialized_header;
        serialized_header.reserve(sizeof(ItlHeader));

        // Opcode (4 bytes)
        serialized_header.insert(serialized_header.end(), header.opcode.begin(), header.opcode.end());

        // ID (4 bytes)
        serialized_header.insert(serialized_header.end(), header.id.begin(), header.id.end());

        // Length (2 bytes)
        serialized_header.insert(serialized_header.end(), header.length.begin(), header.length.end());

        // Counter (2 bytes)
        serialized_header.insert(serialized_header.end(), header.counter.begin(), header.counter.end());

        // Time stamp (4 bytes)
        serialized_header.insert(serialized_header.end(), header.time_stamp.begin(), header.time_stamp.end());

        // Source (1 byte)
        serialized_header.push_back(header.source);

        // Destination (1 byte)
        serialized_header.push_back(header.destination);

        // Checksum (2 bytes)
        serialized_header.insert(serialized_header.end(), header.checksum.begin(), header.checksum.end());

        return serialized_header;
    }

    Result<ItlMessage> ItlProtocol::deserialize(std::span<const std::byte> data) {
        ItlMessage message;
        if (data.size() < sizeof(ItlHeader)) {
            return Result<ItlMessage>::error("Data too short to contain valid header");
        }

        size_t offset = 0;

        // Opcode (4 bytes)
        std::copy_n(data.begin() + offset, 4, message.header.opcode.begin());
        offset += 4;

        // Validate received opcode has 0xF0 in the last byte
        if (message.header.opcode[3] != std::byte{0xF0}) {
            return Result<ItlMessage>::error("Invalid opcode in response");
        }

        // ID (4 bytes)
        std::copy_n(data.begin() + offset, 4, message.header.id.begin());
        offset += 4;

        // Length (2 bytes)
        std::copy_n(data.begin() + offset, 2, message.header.length.begin());
        offset += 2;

        if (const uint16_t length = fromBytes(message.header.length); length != data.size()) {
            return Result<ItlMessage>::error("Length field does not match actual data size");
        }

        // Counter (2 bytes)
        std::copy_n(data.begin() + offset, 2, message.header.counter.begin());
        offset += 2;

        // Time stamp (4 bytes)
        std::copy_n(data.begin() + offset, 4, message.header.time_stamp.begin());
        offset += 4;

        // Source (1 byte)
        message.header.source = data[offset++];

        // Destination (1 byte)
        message.header.destination = data[offset++];

        // Checksum (2 bytes)
        std::copy_n(data.begin() + offset, 2, message.header.checksum.begin());
        offset += 2;

        // Payload (remaining bytes)
        if (offset < data.size()) {
            message.payload.insert(message.payload.end(), data.begin() + offset, data.end());
        }

        if (!isValidChecksum(message)) {
            return Result<ItlMessage>::error("Received message has invalid checksum");
        }

        return Result<ItlMessage>::success(message);
    }

    void ItlProtocol::printMessage(std::span<const std::byte> message) {
        std::string buffer = "Message (" + std::to_string(message.size()) + " bytes): [";
        for (size_t i = 0; i < message.size(); ++i) {
            if (i > 0) buffer += " ";
            char hex_buffer[3];
            snprintf(hex_buffer, sizeof(hex_buffer), "%02x", static_cast<uint8_t>(message[i]));
            buffer += hex_buffer;
        }
        buffer += "]";
        LOG_DEBUG("{}", buffer);
    }

    std::array<std::byte, 2> ItlProtocol::calculateXorChecksum(std::span<const std::byte> data) {
        uint16_t checksum = 0;
        for (const auto byte : data) {
            checksum ^= static_cast<uint8_t>(byte);
        }
        return toBytes(checksum);
    }

    std::array<std::byte, 2> ItlProtocol::calculateMessageChecksum(const ItlMessage& message) {
        auto [header, payload] = message;
        header.checksum = {std::byte{0}, std::byte{0}};

        auto data_for_checksum = serializeHeader(header);
        data_for_checksum.resize(data_for_checksum.size() - 2);
        data_for_checksum.insert(data_for_checksum.end(), message.payload.begin(), message.payload.end());

        return calculateXorChecksum(data_for_checksum);
    }

    bool ItlProtocol::isValidChecksum(const ItlMessage& message) {
        const auto calculated = calculateMessageChecksum(message);
        return calculated == message.header.checksum;
    }

    // Helper functions
    std::array<std::byte, 2> ItlProtocol::toBytes(uint16_t value) {
        return {
            static_cast<std::byte>(value & 0xFF),
            static_cast<std::byte>((value >> 8) & 0xFF)
        };
    }

    uint16_t ItlProtocol::fromBytes(std::span<const std::byte, 2> bytes) {
        return static_cast<uint16_t>(bytes[0]) |
               (static_cast<uint16_t>(bytes[1]) << 8);
    }
} // namespace service::infrastructure
