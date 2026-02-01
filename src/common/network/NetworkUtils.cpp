#include "NetworkUtils.h"

#include <algorithm>
#include <cstring>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>

#include "common/logger/Logger.h"

namespace service::common::network {
    Result<std::vector<NetworkInterface>> getNetworkInterfaces() {
        struct ifaddrs* ifaddr = nullptr;

        if (::getifaddrs(&ifaddr) == -1) {
            return Result<std::vector<NetworkInterface>>::error(
                "Failed to get network interfaces: " + std::string(std::strerror(errno)));
        }

        std::vector<NetworkInterface> interfaces;

        for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) {
                continue;
            }

            // Only process IPv4 addresses
            if (ifa->ifa_addr->sa_family != AF_INET) {
                continue;
            }

            NetworkInterface iface;
            iface.name = ifa->ifa_name;
            iface.is_up = (ifa->ifa_flags & IFF_UP) != 0;
            iface.is_loopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;

            // Get IP address
            auto* addr = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
            char ip_str[INET_ADDRSTRLEN];
            if (::inet_ntop(AF_INET, &addr->sin_addr, ip_str, INET_ADDRSTRLEN) != nullptr) {
                iface.ip_address = ip_str;
                interfaces.push_back(iface);
            }
        }

        ::freeifaddrs(ifaddr);

        if (interfaces.empty()) {
            return Result<std::vector<NetworkInterface>>::error("No network interfaces found");
        }

        return Result<std::vector<NetworkInterface>>::success(interfaces);
    }

    Result<std::string> getPrimaryIpAddress() {
        auto interfaces_result = getNetworkInterfaces();
        if (interfaces_result.isError()) {
            return Result<std::string>::error(interfaces_result.error());
        }

        const auto& interfaces = interfaces_result.value();

        // Filter out loopback and down interfaces
        std::vector<NetworkInterface> valid_interfaces;
        std::copy_if(interfaces.begin(), interfaces.end(), std::back_inserter(valid_interfaces),
                     [](const NetworkInterface& iface) {
                         return !iface.is_loopback && iface.is_up && !iface.ip_address.empty();
                     });

        if (valid_interfaces.empty()) {
            return Result<std::string>::error("No valid non-loopback network interfaces found");
        }

        // Priority: eth0 > ethX > wlan0 > wlanX > others
        auto get_priority = [](const std::string& name) -> int {
            if (name == "eth0") return 0;
            if (name.rfind("eth", 0) == 0) return 1;
            if (name == "wlan0") return 2;
            if (name.rfind("wlan", 0) == 0) return 3;
            if (name.rfind("en", 0) == 0) return 4;  // Modern naming like enp0s3
            return 5;
        };

        std::sort(valid_interfaces.begin(), valid_interfaces.end(),
                  [&get_priority](const NetworkInterface& a, const NetworkInterface& b) {
                      return get_priority(a.name) < get_priority(b.name);
                  });

        const auto& primary = valid_interfaces[0];
        LOG_DEBUG("Selected primary network interface: {} ({})", primary.name, primary.ip_address);

        return Result<std::string>::success(primary.ip_address);
    }

    Result<std::string> getIpAddress(const std::string& interface_name) {
        if (interface_name.empty()) {
            return Result<std::string>::error("Interface name cannot be empty");
        }

        auto interfaces_result = getNetworkInterfaces();
        if (interfaces_result.isError()) {
            return Result<std::string>::error(interfaces_result.error());
        }

        const auto& interfaces = interfaces_result.value();

        for (const auto& iface : interfaces) {
            if (iface.name == interface_name) {
                if (!iface.is_up) {
                    return Result<std::string>::error("Interface " + interface_name + " is down");
                }
                if (iface.ip_address.empty()) {
                    return Result<std::string>::error("Interface " + interface_name + " has no IP address");
                }
                return Result<std::string>::success(iface.ip_address);
            }
        }

        return Result<std::string>::error("Interface " + interface_name + " not found");
    }
} // namespace service::common::network

