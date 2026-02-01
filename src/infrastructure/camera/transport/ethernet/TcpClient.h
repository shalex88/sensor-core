#pragma once

#include <string>

#include "common/types/Result.h"
#include "infrastructure/camera/transport/ITransport.h"

namespace service::infrastructure {
    class TcpClient final : public ITransport {
    public:
        explicit TcpClient(const std::string& device_path);
        ~TcpClient() override;

        Result<void> open() override;
        Result<void> close() override;
        bool isOpen() const override;
        Result<void> write(std::span<const std::byte> tx_data) override;
        Result<size_t> read(std::span<std::byte> rx_data) override;

    private:
        std::string ip_;
        uint16_t port_ = 0;
        int socket_fd_ = -1;
        bool is_connected_ = false;
    };
} // namespace service::infrastructure