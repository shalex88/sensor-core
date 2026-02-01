#include "gtest/gtest.h"
#include "common/network/NetworkUtils.h"

using namespace service::common::network;

class NetworkUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup if needed
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

TEST_F(NetworkUtilsTest, GetNetworkInterfaces_ReturnsNonEmptyList) {
    auto result = getNetworkInterfaces();

    ASSERT_TRUE(result.isSuccess()) << "Failed to get network interfaces: " << result.error();
    EXPECT_FALSE(result.value().empty()) << "Expected at least one network interface";
}

TEST_F(NetworkUtilsTest, GetNetworkInterfaces_ContainsLoopback) {
    auto result = getNetworkInterfaces();

    ASSERT_TRUE(result.isSuccess());
    const auto& interfaces = result.value();

    bool has_loopback = false;
    for (const auto& iface : interfaces) {
        if (iface.is_loopback) {
            has_loopback = true;
            EXPECT_EQ(iface.ip_address, "127.0.0.1") << "Loopback interface should have 127.0.0.1";
            EXPECT_TRUE(iface.name == "lo" || iface.name == "lo0") << "Loopback interface name unexpected: " << iface.name;
            break;
        }
    }

    EXPECT_TRUE(has_loopback) << "Expected to find loopback interface";
}

TEST_F(NetworkUtilsTest, GetNetworkInterfaces_ValidIpAddresses) {
    auto result = getNetworkInterfaces();

    ASSERT_TRUE(result.isSuccess());
    const auto& interfaces = result.value();

    for (const auto& iface : interfaces) {
        EXPECT_FALSE(iface.name.empty()) << "Interface name should not be empty";
        EXPECT_FALSE(iface.ip_address.empty()) << "IP address should not be empty for interface: " << iface.name;

        // Basic validation: IP should contain dots
        EXPECT_NE(iface.ip_address.find('.'), std::string::npos)
            << "IP address should be in IPv4 format: " << iface.ip_address;
    }
}

TEST_F(NetworkUtilsTest, GetPrimaryIpAddress_ReturnsValidIp) {
    auto result = getPrimaryIpAddress();

    ASSERT_TRUE(result.isSuccess()) << "Failed to get primary IP: " << result.error();

    const auto& ip = result.value();
    EXPECT_FALSE(ip.empty());
    EXPECT_NE(ip, "127.0.0.1") << "Primary IP should not be loopback";
    EXPECT_NE(ip.find('.'), std::string::npos) << "IP should be in IPv4 format";
}

TEST_F(NetworkUtilsTest, GetPrimaryIpAddress_PrefersEth0) {
    auto interfaces_result = getNetworkInterfaces();
    ASSERT_TRUE(interfaces_result.isSuccess());

    const auto& interfaces = interfaces_result.value();
    bool has_eth0 = false;
    bool has_other_non_loopback = false;

    for (const auto& iface : interfaces) {
        if (iface.name == "eth0" && iface.is_up && !iface.is_loopback) {
            has_eth0 = true;
        } else if (!iface.is_loopback && iface.is_up && iface.name != "eth0") {
            has_other_non_loopback = true;
        }
    }

    // If eth0 exists and is up, it should be selected
    if (has_eth0) {
        auto result = getPrimaryIpAddress();
        ASSERT_TRUE(result.isSuccess());

        // Find eth0's IP
        std::string eth0_ip;
        for (const auto& iface : interfaces) {
            if (iface.name == "eth0") {
                eth0_ip = iface.ip_address;
                break;
            }
        }

        EXPECT_EQ(result.value(), eth0_ip) << "Should prefer eth0 when available";
    }
}

TEST_F(NetworkUtilsTest, GetIpAddress_WithValidInterface) {
    // First get all interfaces
    auto interfaces_result = getNetworkInterfaces();
    ASSERT_TRUE(interfaces_result.isSuccess());

    const auto& interfaces = interfaces_result.value();
    ASSERT_FALSE(interfaces.empty());

    // Test with the first interface
    const auto& first_iface = interfaces[0];
    auto result = getIpAddress(first_iface.name);

    if (first_iface.is_up) {
        ASSERT_TRUE(result.isSuccess()) << "Failed to get IP for interface: " << first_iface.name;
        EXPECT_EQ(result.value(), first_iface.ip_address);
    }
}

TEST_F(NetworkUtilsTest, GetIpAddress_WithLoopback) {
    auto result = getIpAddress("lo");

    // On most Linux systems, "lo" should exist
    if (result.isSuccess()) {
        EXPECT_EQ(result.value(), "127.0.0.1");
    }
}

TEST_F(NetworkUtilsTest, GetIpAddress_WithInvalidInterface) {
    auto result = getIpAddress("nonexistent_interface_xyz");

    ASSERT_TRUE(result.isError());
    EXPECT_NE(result.error().find("not found"), std::string::npos);
}

TEST_F(NetworkUtilsTest, GetIpAddress_WithEmptyInterfaceName) {
    auto result = getIpAddress("");

    ASSERT_TRUE(result.isError());
    EXPECT_NE(result.error().find("empty"), std::string::npos);
}

TEST_F(NetworkUtilsTest, NetworkInterface_HasAllFields) {
    auto result = getNetworkInterfaces();
    ASSERT_TRUE(result.isSuccess());

    const auto& interfaces = result.value();
    ASSERT_FALSE(interfaces.empty());

    for (const auto& iface : interfaces) {
        // Verify all fields are populated/initialized
        EXPECT_FALSE(iface.name.empty());
        EXPECT_FALSE(iface.ip_address.empty());
        // is_up and is_loopback are bools, so just access them
        [[maybe_unused]] bool up = iface.is_up;
        [[maybe_unused]] bool loopback = iface.is_loopback;
    }
}

TEST_F(NetworkUtilsTest, GetPrimaryIpAddress_ConsistentResults) {
    // Call multiple times and verify consistent results
    auto result1 = getPrimaryIpAddress();
    auto result2 = getPrimaryIpAddress();

    ASSERT_TRUE(result1.isSuccess());
    ASSERT_TRUE(result2.isSuccess());

    EXPECT_EQ(result1.value(), result2.value())
        << "Primary IP should be consistent across calls";
}

TEST_F(NetworkUtilsTest, InterfacePriority_EthBeforeWlan) {
    auto interfaces_result = getNetworkInterfaces();
    ASSERT_TRUE(interfaces_result.isSuccess());

    const auto& interfaces = interfaces_result.value();

    bool has_eth = false;
    bool has_wlan = false;

    for (const auto& iface : interfaces) {
        if (!iface.is_loopback && iface.is_up) {
            if (iface.name.rfind("eth", 0) == 0) has_eth = true;
            if (iface.name.rfind("wlan", 0) == 0) has_wlan = true;
        }
    }

    // If both eth and wlan exist, eth should be preferred
    if (has_eth && has_wlan) {
        auto result = getPrimaryIpAddress();
        ASSERT_TRUE(result.isSuccess());

        // Find which interface was selected
        std::string selected_interface;
        for (const auto& iface : interfaces) {
            if (iface.ip_address == result.value()) {
                selected_interface = iface.name;
                break;
            }
        }

        EXPECT_TRUE(selected_interface.rfind("eth", 0) == 0)
            << "Should prefer eth over wlan, but selected: " << selected_interface;
    }
}

