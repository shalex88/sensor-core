#include <gmock/gmock.h>
#include <gtest/gtest.h>
/* Add your project include files here */
#include "common/config/ConfigManager.h"
#include "common/logger/Logger.h"
#include "common/types/Result.h"
#include "infrastructure/camera/devices/SonyCamera.h"
#include "infrastructure/camera/hal/Camera.h"
#include "infrastructure/camera/protocol/visca/ViscaProtocol.h"
#include "infrastructure/camera/transport/uart/Uart.h"

using namespace service;
using namespace testing;

class SonyCameraTests : public Test {
protected:
    SonyCameraTests() : config_(std::make_unique<common::ConfigManager>("../../config/config-wfov.yaml")) {
        CONFIGURE_LOGGER(config_->getAppName(), config_->getLogLevel());
        const auto& infrastructure_config = config_->getInfrastructureConfig();
        const auto& endpoint = infrastructure_config.endpoints[0];
        auto uart = std::make_unique<infrastructure::Uart>(
            endpoint.address,
            endpoint.configuration.at("baud_rate"));
        auto protocol = std::make_unique<infrastructure::ViscaProtocol>(std::move(uart));
        auto camera_hw = std::make_unique<infrastructure::SonyCamera>(std::move(protocol));
        camera_ = std::make_unique<infrastructure::Camera>(std::move(camera_hw));
    }

    std::unique_ptr<infrastructure::Camera> camera_;
    std::unique_ptr<common::ConfigManager> config_;
};

// ============================================================================
// Construction and Basic Operations
// ============================================================================

TEST_F(SonyCameraTests, CanBeConstructed) {
    const auto& infrastructure_config = config_->getInfrastructureConfig();
    const auto& endpoint = infrastructure_config.endpoints[0];
    auto uart = std::make_unique<infrastructure::Uart>(
        endpoint.address,
        endpoint.configuration.at("baud_rate"));
    auto protocol = std::make_unique<infrastructure::ViscaProtocol>(std::move(uart));
    const auto camera = std::make_unique<infrastructure::SonyCamera>(std::move(protocol));
    ASSERT_NE(nullptr, camera);
}

TEST_F(SonyCameraTests, InitiallyNotConnected) {
    EXPECT_FALSE(camera_->isConnected());
}

TEST_F(SonyCameraTests, ConnectDisconnect) {
    const auto connect_result = camera_->open();
    ASSERT_TRUE(connect_result.isSuccess()) << "Failed to connect: " << connect_result.error();
    EXPECT_TRUE(camera_->isConnected());

    const auto disconnect_result = camera_->close();
    ASSERT_TRUE(disconnect_result.isSuccess()) << "Failed to disconnect: " << disconnect_result.error();
    EXPECT_FALSE(camera_->isConnected());
}

TEST_F(SonyCameraTests, CanBeConnected) {
    const auto result = camera_->open();
    ASSERT_TRUE(result.isSuccess()) << "Failed to connect: " << result.error();
    EXPECT_TRUE(camera_->isConnected());
}

TEST_F(SonyCameraTests, DisconnectWhenNotConnectedSucceeds) {
    // Should succeed even if not connected
    const auto result = camera_->close();
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(SonyCameraTests, ReconnectAfterDisconnect) {
    // First connection
    ASSERT_TRUE(camera_->open().isSuccess());
    EXPECT_TRUE(camera_->isConnected());

    // Disconnect
    ASSERT_TRUE(camera_->close().isSuccess());
    EXPECT_FALSE(camera_->isConnected());

    // Reconnect
    ASSERT_TRUE(camera_->open().isSuccess());
    EXPECT_TRUE(camera_->isConnected());
}

// ============================================================================
// Zoom Operations
// ============================================================================

TEST_F(SonyCameraTests, ZoomOperationsBasic) {
    ASSERT_TRUE(camera_->open().isSuccess());

    constexpr auto test_zoom = 5u;

    const auto set_result = camera_->setZoom(test_zoom);
    ASSERT_TRUE(set_result.isSuccess()) << "Failed to set zoom: " << set_result.error();

    // Give camera time to adjust
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto get_result = camera_->getZoom();
    ASSERT_TRUE(get_result.isSuccess()) << "Failed to get zoom: " << get_result.error();
    EXPECT_EQ(test_zoom, get_result.value());
}

TEST_F(SonyCameraTests, ZoomOperationsMultipleValues) {
    ASSERT_TRUE(camera_->open().isSuccess());

    const std::vector<common::types::zoom> test_values = {0, 10, 20, 50, 100};

    for (const auto zoom : test_values) {
        const auto set_result = camera_->setZoom(zoom);
        ASSERT_TRUE(set_result.isSuccess()) << "Failed to set zoom " << zoom << ": " << set_result.error();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        const auto get_result = camera_->getZoom();
        ASSERT_TRUE(get_result.isSuccess()) << "Failed to get zoom: " << get_result.error();
        EXPECT_EQ(zoom, get_result.value()) << "Zoom mismatch for value " << zoom;
    }
}

TEST_F(SonyCameraTests, ZoomOperationsMinValue) {
    ASSERT_TRUE(camera_->open().isSuccess());

    constexpr auto min_zoom = 0u;

    ASSERT_TRUE(camera_->setZoom(min_zoom).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto get_result = camera_->getZoom();
    ASSERT_TRUE(get_result.isSuccess());
    EXPECT_EQ(min_zoom, get_result.value());
}

TEST_F(SonyCameraTests, ZoomOperationsMaxValue) {
    ASSERT_TRUE(camera_->open().isSuccess());

    constexpr auto max_zoom = 100u;

    ASSERT_TRUE(camera_->setZoom(max_zoom).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto get_result = camera_->getZoom();
    ASSERT_TRUE(get_result.isSuccess());
    EXPECT_EQ(max_zoom, get_result.value());
}

TEST_F(SonyCameraTests, ZoomOperationsFailWhenNotConnected) {
    constexpr auto test_zoom = 50u;

    const auto set_result = camera_->setZoom(test_zoom);
    ASSERT_TRUE(set_result.isError());
    EXPECT_THAT(set_result.error(), HasSubstr("not connected"));

    const auto get_result = camera_->getZoom();
    ASSERT_TRUE(get_result.isError());
    EXPECT_THAT(get_result.error(), HasSubstr("not connected"));
}

// ============================================================================
// Focus Operations
// ============================================================================

TEST_F(SonyCameraTests, FocusOperationsBasic) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Disable autofocus first
    ASSERT_TRUE(camera_->enableAutoFocus(false).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    constexpr auto test_focus = 30u;

    const auto set_result = camera_->setFocus(test_focus);
    ASSERT_TRUE(set_result.isSuccess()) << "Failed to set focus: " << set_result.error();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto get_result = camera_->getFocus();
    ASSERT_TRUE(get_result.isSuccess()) << "Failed to get focus: " << get_result.error();
    EXPECT_EQ(test_focus, get_result.value());
}

TEST_F(SonyCameraTests, FocusOperationsMultipleValues) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Disable autofocus first
    ASSERT_TRUE(camera_->enableAutoFocus(false).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const std::vector<common::types::focus> test_values = {10, 20, 30, 50, 80};

    for (const auto focus : test_values) {
        const auto set_result = camera_->setFocus(focus);
        ASSERT_TRUE(set_result.isSuccess()) << "Failed to set focus " << focus << ": " << set_result.error();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        const auto get_result = camera_->getFocus();
        ASSERT_TRUE(get_result.isSuccess()) << "Failed to get focus: " << get_result.error();
        EXPECT_EQ(focus, get_result.value()) << "Focus mismatch for value " << focus;
    }
}

TEST_F(SonyCameraTests, FocusOperationsFailWhenNotConnected) {
    constexpr auto test_focus = 50u;

    const auto set_result = camera_->setFocus(test_focus);
    ASSERT_TRUE(set_result.isError());
    EXPECT_THAT(set_result.error(), HasSubstr("not connected"));

    const auto get_result = camera_->getFocus();
    ASSERT_TRUE(get_result.isError());
    EXPECT_THAT(get_result.error(), HasSubstr("not connected"));
}

TEST_F(SonyCameraTests, FocusOperationsFailWhenAutoFocusEnabled) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Enable autofocus
    ASSERT_TRUE(camera_->enableAutoFocus(true).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    constexpr auto test_focus = 50u;

    // Should fail because autofocus is enabled
    const auto set_result = camera_->setFocus(test_focus);
    ASSERT_TRUE(set_result.isError());
    EXPECT_THAT(set_result.error(), HasSubstr("autofocus"));

    const auto get_result = camera_->getFocus();
    ASSERT_TRUE(get_result.isError());
    EXPECT_THAT(get_result.error(), HasSubstr("autofocus"));
}

// ============================================================================
// AutoFocus Operations
// ============================================================================

TEST_F(SonyCameraTests, AutoFocusEnableDisable) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Enable autofocus
    const auto enable_result = camera_->enableAutoFocus(true);
    ASSERT_TRUE(enable_result.isSuccess()) << "Failed to enable autofocus: " << enable_result.error();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Disable autofocus
    const auto disable_result = camera_->enableAutoFocus(false);
    ASSERT_TRUE(disable_result.isSuccess()) << "Failed to disable autofocus: " << disable_result.error();
}

TEST_F(SonyCameraTests, AutoFocusMultipleToggles) {
    ASSERT_TRUE(camera_->open().isSuccess());

    for (int i = 0; i < 3; ++i) {
        ASSERT_TRUE(camera_->enableAutoFocus(true).isSuccess()) << "Failed to enable autofocus iteration " << i;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        ASSERT_TRUE(camera_->enableAutoFocus(false).isSuccess()) << "Failed to disable autofocus iteration " << i;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

TEST_F(SonyCameraTests, AutoFocusFailWhenNotConnected) {
    const auto result = camera_->enableAutoFocus(true);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), HasSubstr("not connected"));
}

// ============================================================================
// Stabilization Operations
// ============================================================================

TEST_F(SonyCameraTests, StabilizationEnableDisable) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Enable stabilization
    const auto enable_result = camera_->stabilize(true);
    ASSERT_TRUE(enable_result.isSuccess()) << "Failed to enable stabilization: " << enable_result.error();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Disable stabilization
    const auto disable_result = camera_->stabilize(false);
    ASSERT_TRUE(disable_result.isSuccess()) << "Failed to disable stabilization: " << disable_result.error();
}

TEST_F(SonyCameraTests, StabilizationMultipleToggles) {
    ASSERT_TRUE(camera_->open().isSuccess());

    for (int i = 0; i < 3; ++i) {
        ASSERT_TRUE(camera_->stabilize(true).isSuccess()) << "Failed to enable stabilization iteration " << i;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        ASSERT_TRUE(camera_->stabilize(false).isSuccess()) << "Failed to disable stabilization iteration " << i;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

TEST_F(SonyCameraTests, StabilizationFailWhenNotConnected) {
    const auto result = camera_->stabilize(true);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), HasSubstr("not connected"));
}

// ============================================================================
// Info Operations
// ============================================================================

TEST_F(SonyCameraTests, GetInfoSuccess) {
    ASSERT_TRUE(camera_->open().isSuccess());

    const auto result = camera_->getInfo();
    ASSERT_TRUE(result.isSuccess()) << "Failed to get info: " << result.error();
    EXPECT_FALSE(result.value().empty()) << "Info string should not be empty";
}

TEST_F(SonyCameraTests, GetInfoFailWhenNotConnected) {
    const auto result = camera_->getInfo();
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), HasSubstr("not connected"));
}

// ============================================================================
// Combined Operations and Edge Cases
// ============================================================================

TEST_F(SonyCameraTests, CombinedZoomAndFocusOperations) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Disable autofocus
    ASSERT_TRUE(camera_->enableAutoFocus(false).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Set zoom
    constexpr auto test_zoom = 25u;
    ASSERT_TRUE(camera_->setZoom(test_zoom).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Set focus
    constexpr auto test_focus = 40u;
    ASSERT_TRUE(camera_->setFocus(test_focus).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Verify both values
    const auto zoom_result = camera_->getZoom();
    ASSERT_TRUE(zoom_result.isSuccess());
    EXPECT_EQ(test_zoom, zoom_result.value());

    const auto focus_result = camera_->getFocus();
    ASSERT_TRUE(focus_result.isSuccess());
    EXPECT_EQ(test_focus, focus_result.value());
}

TEST_F(SonyCameraTests, RapidZoomChanges) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Rapidly change zoom values
    for (common::types::zoom zoom = 0; zoom <= 100; zoom += 20) {
        ASSERT_TRUE(camera_->setZoom(zoom).isSuccess()) << "Failed at zoom " << zoom;
        // Small delay between commands
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

TEST_F(SonyCameraTests, StressTestAllOperations) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Disable autofocus for focus operations
    ASSERT_TRUE(camera_->enableAutoFocus(false).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test multiple operations in sequence
    for (int iteration = 0; iteration < 3; ++iteration) {
        // Zoom
        ASSERT_TRUE(camera_->setZoom(10 + iteration * 10).isSuccess());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Focus
        ASSERT_TRUE(camera_->setFocus(20 + iteration * 10).isSuccess());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Stabilization
        ASSERT_TRUE(camera_->stabilize(iteration % 2 == 0).isSuccess());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Get info
        ASSERT_TRUE(camera_->getInfo().isSuccess());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// ============================================================================
// Limits and Boundary Tests
// ============================================================================

TEST_F(SonyCameraTests, ZoomAtBoundaries) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Test minimum zoom (0 is normalized minimum)
    constexpr common::types::zoom min_zoom = 0u;

    // Test minimum
    ASSERT_TRUE(camera_->setZoom(min_zoom).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    const auto min_result = camera_->getZoom();
    ASSERT_TRUE(min_result.isSuccess());
    EXPECT_EQ(min_zoom, min_result.value());

    // Test maximum (100 is normalized maximum)
    constexpr common::types::zoom max_zoom = 100u;
    ASSERT_TRUE(camera_->setZoom(max_zoom).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    const auto max_result = camera_->getZoom();
    ASSERT_TRUE(max_result.isSuccess());
    EXPECT_EQ(max_zoom, max_result.value());
}

// ============================================================================
// Error Recovery Tests
// ============================================================================

TEST_F(SonyCameraTests, RecoverFromInvalidOperation) {
    ASSERT_TRUE(camera_->open().isSuccess());

    // Enable autofocus
    ASSERT_TRUE(camera_->enableAutoFocus(true).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Try invalid focus operation (setFocus should fail when autofocus is enabled)
    const auto invalid_result = camera_->setFocus(50);
    ASSERT_TRUE(invalid_result.isError());

    // Disable autofocus and retry
    ASSERT_TRUE(camera_->enableAutoFocus(false).isSuccess());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Should work now
    const auto valid_result = camera_->setFocus(50);
    ASSERT_TRUE(valid_result.isSuccess()) << "Failed to recover: " << valid_result.error();
}
