#include "FakeUart.h"

#include "common/logger/Logger.h"

namespace service::infrastructure {
    FakeUart::FakeUart() {
        if (open().isError()) {
            LOG_ERROR("Failed to open Fake UART interface");
        }
    }

    FakeUart::~FakeUart() {
        if (close().isError()) {
            LOG_ERROR("Failed to close Fake UART interface");
        }
    }

    Result<void> FakeUart::open() {
        std::unique_lock lock(data_mutex_);
        if (is_open_) {
            return Result<void>::error("Fake UART device is already open");
        }

        is_open_ = true;
        stored_data_.clear();

        return Result<void>::success();
    }

    Result<void> FakeUart::close() {
        std::unique_lock lock(data_mutex_);
        if (!is_open_) {
            return Result<void>::success();
        }

        is_open_ = false;
        stored_data_.clear();

        return Result<void>::success();
    }

    Result<void> FakeUart::configure(const int baud_rate, const int data_bits, const int stop_bits,
                                     const char parity) const {
        std::shared_lock lock(data_mutex_);
        if (!is_open_) {
            return Result<void>::error("Fake UART device is not open");
        }

        LOG_DEBUG("Fake UART configured: {}bps, {}{}{}", baud_rate, data_bits, parity, stop_bits);
        return Result<void>::success();
    }

    Result<void> FakeUart::write(std::span<const std::byte> tx_data) {
        std::unique_lock lock(data_mutex_);
        if (!is_open_) {
            return Result<void>::error("Fake UART device is not open");
        }

        stored_data_.assign(tx_data.begin(), tx_data.end());
        return Result<void>::success();
    }

    Result<size_t> FakeUart::read(const std::span<std::byte> rx_data) {
        std::shared_lock lock(data_mutex_);
        if (!is_open_) {
            return Result<size_t>::error("Fake UART device is not open");
        }

        return Result<void>::success(rx_data.size());
    }

    bool FakeUart::isOpen() const {
        std::shared_lock lock(data_mutex_);
        return is_open_;
    }
} // namespace service::infrastructure