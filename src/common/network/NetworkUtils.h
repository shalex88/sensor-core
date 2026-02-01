#pragma once

#include <string>
#include <vector>

#include "common/types/Result.h"

namespace service::common::network {
    /**
     * Represents a network interface with its properties
     */
    struct NetworkInterface {
        std::string name;        // e.g., "eth0", "wlan0"
        std::string ip_address;  // IPv4 address
        bool is_up;
        bool is_loopback;
    };

    /**
     * Get all network interfaces on the system
     * @return Result containing vector of interfaces or error message
     */
    Result<std::vector<NetworkInterface>> getNetworkInterfaces();

    /**
     * Get the primary non-loopback IPv4 address of this device
     * Selection priority: eth0 > wlan0 > other non-loopback interfaces
     * @return Result containing IP address string or error message
     */
    Result<std::string> getPrimaryIpAddress();

    /**
     * Get IP address of a specific network interface
     * @param interface_name Name of the interface (e.g., "eth0")
     * @return Result containing IP address string or error message
     */
    Result<std::string> getIpAddress(const std::string& interface_name);
} // namespace service::common::network

