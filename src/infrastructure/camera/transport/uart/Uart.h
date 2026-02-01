#pragma once

#include <string>
#include <string_view>
#include <termios.h>

#include "infrastructure/camera/transport/ITransport.h"

namespace service::infrastructure {
    class Uart final : public ITransport {
    public:
        explicit Uart(std::string device_path, std::string_view baud_rate);
        ~Uart() override;

        Result<void> open() override;
        Result<void> close() override;
        Result<void> write(std::span<const std::byte> data) override;
        Result<size_t> read(std::span<std::byte> rx_data) override;
        bool isOpen() const override;

    private:
        std::string device_path_;
        int port_fd_{-1};
        termios options_{};
        speed_t baud_rate_{};
    };
} // namespace service::infrastructure