#pragma once

#include <shared_mutex>
#include <vector>

#include "infrastructure/camera/transport/ITransport.h"

namespace service::infrastructure {
    class FakeUart final : public ITransport {
    public:
        FakeUart();
        ~FakeUart() override;

        Result<void> open() override;
        Result<void> close() override;
        Result<void> write(std::span<const std::byte> tx_data) override;
        Result<size_t> read(std::span<std::byte> rx_data) override;
        bool isOpen() const override;
        Result<void> configure(int baud_rate = 115200, int data_bits = 8, int stop_bits = 1, char parity = 'N') const;

    private:
        bool is_open_ = false;
        std::vector<std::byte> stored_data_;
        mutable std::shared_mutex data_mutex_;
    };
} // namespace service::infrastructure