#include "TcpClient.h"

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>

namespace service::infrastructure {
    TcpClient::TcpClient(const std::string& device_path) {
        if (device_path.empty()) {
            throw std::invalid_argument("Device path cannot be empty");
        }

        const auto colon_pos = device_path.find(':');
        if (colon_pos == std::string::npos || colon_pos == 0 || colon_pos == device_path.size() - 1) {
            throw std::invalid_argument("Device path must be in format <ip>:<port>");
        }
        ip_ = device_path.substr(0, colon_pos);
        const std::string port_str = device_path.substr(colon_pos + 1);
        try {
            const int port_val = std::stoi(port_str);
            if (port_val < 1 || port_val > 65535) {
                throw std::out_of_range("Port out of range");
            }
            port_ = static_cast<uint16_t>(port_val);
        } catch (const std::exception& e) {
            throw std::invalid_argument(std::string("Invalid port in device path: ") + e.what());
        }

        if (open().isError()) {
            throw std::runtime_error("Failed to open TCP client transport");
        }
    }

    TcpClient::~TcpClient() {
        if (close().isError()) {
            LOG_ERROR("Failed to close TCP client transport");
        }
    }

    Result<void> TcpClient::open() {
        if (is_connected_) {
            return Result<void>::success();
        }
        socket_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ < 0) {
            return Result<void>::error("Failed to create socket");
        }

        // Set socket to non-blocking mode for connection timeout
        int flags = ::fcntl(socket_fd_, F_GETFL, 0);
        if (flags < 0 || ::fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
            ::close(socket_fd_);
            socket_fd_ = -1;
            return Result<void>::error("Failed to set socket non-blocking");
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        if (::inet_pton(AF_INET, ip_.c_str(), &server_addr.sin_addr) <= 0) {
            ::close(socket_fd_);
            socket_fd_ = -1;
            return Result<void>::error("Invalid address");
        }

        // Attempt connection (will return immediately due to non-blocking)
        int connect_result = ::connect(socket_fd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

        if (connect_result < 0) {
            if (errno == EINPROGRESS) {
                // Connection in progress, wait with timeout (3 seconds)
                fd_set write_fds;
                FD_ZERO(&write_fds);
                FD_SET(socket_fd_, &write_fds);

                struct timeval timeout;
                timeout.tv_sec = 3;
                timeout.tv_usec = 0;

                int select_result = ::select(socket_fd_ + 1, nullptr, &write_fds, nullptr, &timeout);

                if (select_result <= 0) {
                    // Timeout or error
                    ::close(socket_fd_);
                    socket_fd_ = -1;
                    return Result<void>::error(select_result == 0 ?
                        "Connection timeout (address not reachable)" :
                        "Connection failed: " + std::string(std::strerror(errno)));
                }

                // Check if connection was successful
                int error = 0;
                socklen_t len = sizeof(error);
                if (::getsockopt(socket_fd_, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                    ::close(socket_fd_);
                    socket_fd_ = -1;
                    return Result<void>::error("Connection failed: " + std::string(std::strerror(error ? error : errno)));
                }
            } else {
                // Immediate connection error
                ::close(socket_fd_);
                socket_fd_ = -1;
                return Result<void>::error("Connection failed: " + std::string(std::strerror(errno)));
            }
        }

        // Restore blocking mode
        if (::fcntl(socket_fd_, F_SETFL, flags) < 0) {
            ::close(socket_fd_);
            socket_fd_ = -1;
            return Result<void>::error("Failed to restore socket blocking mode");
        }

        is_connected_ = true;
        return Result<void>::success();
    }

    Result<void> TcpClient::close() {
        if (is_connected_ && socket_fd_ >= 0) {
            ::close(socket_fd_);
            socket_fd_ = -1;
            is_connected_ = false;
        }
        return Result<void>::success();
    }

    bool TcpClient::isOpen() const {
        if (!is_connected_ || socket_fd_ < 0) {
            return false;
        }
        return true;
    }

    Result<void> TcpClient::write(const std::span<const std::byte> tx_data) {
        if (!isOpen()) {
            return Result<void>::error("Not connected");
        }

        size_t total_sent = 0;

        while (total_sent < tx_data.size()) {
            const ssize_t sent = ::send(socket_fd_, tx_data.data() + total_sent, tx_data.size() - total_sent, 0);
            if (sent < 0) {
                return Result<void>::error("Send failed");
            }
            total_sent += static_cast<size_t>(sent);
        }
        return Result<void>::success();
    }

    Result<size_t> TcpClient::read(std::span<std::byte> rx_data) {
        if (!isOpen()) {
            return Result<size_t>::error("Not connected");
        }

        while (true) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(socket_fd_, &read_fds);

            // reinitialize timeout each select call because select may modify it
            timeval timeout{1, 0};

            const auto select_result = ::select(socket_fd_ + 1, &read_fds, nullptr, nullptr, &timeout);
            if (select_result < 0) {
                if (errno == EINTR) {
                    // interrupted by signal, retry
                    continue;
                }
                return Result<size_t>::error(std::string("Select failed: ") + std::strerror(errno));
            }
            if (select_result == 0) {
                return Result<size_t>::error("Read timeout");
            }

            if (!FD_ISSET(socket_fd_, &read_fds)) {
                // unexpected: select reported activity but socket not set
                return Result<size_t>::error("Select returned without socket readiness");
            }

            const ssize_t bytes_read = ::recv(socket_fd_, rx_data.data(), rx_data.size(), 0);
            if (bytes_read < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // try again (non-blocking case)
                    continue;
                }
                return Result<size_t>::error(
                    std::string("Failed to receive from TCP: ") + std::strerror(errno));
            }
            if (bytes_read == 0) {
                // peer performed orderly shutdown
                return Result<size_t>::error("Connection closed by peer");
            }

            return Result<void>::success(static_cast<size_t>(bytes_read));
        }
    }
} // namespace service::infrastructure