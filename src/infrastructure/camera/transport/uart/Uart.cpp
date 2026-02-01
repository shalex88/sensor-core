#include "Uart.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <string_view>
#include <termios.h>
#include <unistd.h>
#include <utility>
#include <sys/select.h>

#include "common/logger/Logger.h"

namespace service::infrastructure {
    namespace {
        [[nodiscard]] speed_t toTermiosBaud(const std::string_view baud_rate) {
            if (baud_rate == "9600") {
                return B9600;
            }
            if (baud_rate == "19200") {
                return B19200;
            }
            if (baud_rate == "38400") {
                return B38400;
            }
            if (baud_rate == "57600") {
                return B57600;
            }
            if (baud_rate == "115200") {
                return B115200;
            }

            throw std::invalid_argument("Invalid baud rate: " + std::string(baud_rate) +
                                      ". Supported: 9600, 19200, 38400, 57600, 115200");
        }

        std::string getHexDump(const std::span<const std::byte> data) {
            if (data.empty()) {
                return "[]";
            }

            std::string result = "[";
            for (size_t i = 0; i < data.size(); ++i) {
                char buf[8];
                snprintf(buf, sizeof(buf), "0x%02X", static_cast<unsigned char>(data[i]));
                result += buf;
                if (i < data.size() - 1) {
                    result += ", ";
                }
            }
            result += "]";
            return result;
        }
    }

    Uart::Uart(std::string device_path, const std::string_view baud_rate)
        : device_path_(std::move(device_path)) {
        if (device_path_.empty()) {
            throw std::invalid_argument("UART device path cannot be empty");
        }
        if (baud_rate.empty()) {
            throw std::invalid_argument("UART baud rate cannot be empty");
        }
        baud_rate_ = toTermiosBaud(baud_rate);
    }

    Uart::~Uart() {
        if (close().isError()) {
            LOG_ERROR("Failed to close UART device: {}", device_path_);
        }
    }

    Result<void> Uart::open() {
        if (isOpen()) {
            return Result<void>::error("UART device is already open");
        }

        const auto fd = ::open(device_path_.c_str(), O_RDWR | O_NDELAY | O_NOCTTY);
        if (fd < 0) {
            return Result<void>::error(
                "Failed to open UART device: " + device_path_ + " - " + std::string(strerror(errno)));
        }

        fcntl(fd, F_SETFL, 0);
        tcgetattr(fd, &options_);

        cfsetispeed(&options_, baud_rate_);
        cfsetospeed(&options_, baud_rate_);
        options_.c_cflag &= ~PARENB;
        options_.c_cflag &= ~CSTOPB;
        options_.c_cflag &= ~CSIZE;
        options_.c_cflag |= CS8;
        options_.c_cflag &= ~CRTSCTS;
        options_.c_cc[VMIN]  = 10; //FIXME: bad solution, needed for mpsoc
        options_.c_cc[VTIME] = 1; //FIXME: bad solution, needed for mpsoc

        options_.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

        options_.c_iflag = 0;

        options_.c_oflag &= ~OPOST;

        tcsetattr(fd, TCSANOW, &options_);
        port_fd_ = fd;

        return Result<void>::success();
    }

    Result<void> Uart::close() {
        if (!isOpen()) {
            return Result<void>::success();
        }

        if (::close(port_fd_) < 0) {
            return Result<void>::error("Failed to close UART device: " + std::string(strerror(errno)));
        }

        port_fd_ = -1;
        return Result<void>::success();
    }

    Result<void> Uart::write(const std::span<const std::byte> data) {
        if (!isOpen()) {
            return Result<void>::error("UART device is not open");
        }

        if (data.empty()) {
            return Result<void>::success();
        }

        LOG_TRACE("UART TX: {}", getHexDump(data));

        if (const auto bytes_written = ::write(port_fd_, data.data(), data.size()); bytes_written < 0) {
            return Result<void>::error("Failed to write to UART: " + std::string(strerror(errno)));
        }

        return Result<void>::success();
    }

    Result<size_t> Uart::read(std::span<std::byte> rx_data) {
        if (!isOpen()) {
            return Result<size_t>::error("UART device is not open");
        }

        while (true) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(port_fd_, &read_fds);

            timeval timeout{10, 0};

            const auto select_result = ::select(port_fd_ + 1, &read_fds, nullptr, nullptr, &timeout);
            if (select_result < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return Result<size_t>::error(std::string("Select failed: ") + std::strerror(errno));
            }
            if (select_result == 0) {
                return Result<size_t>::error("Read timeout");
            }

            if (!FD_ISSET(port_fd_, &read_fds)) {
                return Result<size_t>::error("Select returned without UART readiness");
            }

            const ssize_t bytes_read = ::read(port_fd_, rx_data.data(), rx_data.size());
            if (bytes_read < 0) {
                if (errno == EINTR) {
                    continue;
                }
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                }
                return Result<size_t>::error(std::string("Failed to read from UART: ") + std::strerror(errno));
            }

            if (bytes_read == 0) {
                return Result<size_t>::error("UART device closed");
            }

            LOG_TRACE("UART RX: {}", getHexDump(rx_data.subspan(0, static_cast<size_t>(bytes_read))));

            return Result<size_t>::success(static_cast<size_t>(bytes_read));
        }
    }

    bool Uart::isOpen() const {
        return port_fd_ > 0;
    }
} // namespace service::infrastructure