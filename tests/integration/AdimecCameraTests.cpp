#include <gmock/gmock.h>
#include <gtest/gtest.h>
/* Add your project include files here */
#include "common/config/ConfigManager.h"
#include "common/logger/Logger.h"
#include "common/types/Result.h"
#include "infrastructure/camera/devices/AdimecCamera.h"
#include "infrastructure/camera/hal/Camera.h"
#include "infrastructure/camera/protocol/genicam/FpgaTransport.h"
#include "infrastructure/camera/protocol/genicam/GenicamProtocol.h"
#include "infrastructure/camera/protocol/itl/ItlProtocol.h"
#include "infrastructure/camera/transport/ethernet/TcpClient.h"

using namespace service;
using namespace testing;

class AdimecCameraTests : public Test {
protected:
    AdimecCameraTests() : config_(std::make_unique<common::ConfigManager>("../../config/config-nfov.yaml")) {
        CONFIGURE_LOGGER(config_->getAppName(), config_->getLogLevel());
        const auto& infrastructure_config = config_->getInfrastructureConfig();
        const auto& camera_endpoint = infrastructure_config.endpoints[0];
        const auto& lens_endpoint = infrastructure_config.endpoints[1];
        auto camera_transport = std::make_unique<infrastructure::FpgaTransport>(camera_endpoint.address);
        auto camera_protocol = std::make_unique<infrastructure::GenicamProtocol>(std::move(camera_transport));
        auto lens_transport = std::make_unique<infrastructure::TcpClient>(lens_endpoint.address);
        auto lens_protocol = std::make_unique<infrastructure::ItlProtocol>(std::move(lens_transport));
        auto camera_hw = std::make_unique<infrastructure::AdimecCamera>(std::move(camera_protocol), std::move(lens_protocol));
        camera_ = std::make_unique<infrastructure::Camera>(std::move(camera_hw));
    }

    std::unique_ptr<infrastructure::Camera> camera_;
    std::unique_ptr<common::ConfigManager> config_;
};

TEST_F(AdimecCameraTests, CanBeConstructed) {
    const auto& infrastructure_config = config_->getInfrastructureConfig();
    const auto& camera_endpoint = infrastructure_config.endpoints[0];
    const auto& lens_endpoint = infrastructure_config.endpoints[1];
    auto camera_transport = std::make_unique<infrastructure::FpgaTransport>(camera_endpoint.address);
    auto camera_protocol = std::make_unique<infrastructure::GenicamProtocol>(std::move(camera_transport));
    auto lens_transport = std::make_unique<infrastructure::TcpClient>(lens_endpoint.address);
    auto lens_protocol = std::make_unique<infrastructure::ItlProtocol>(std::move(lens_transport));
    const auto camera = std::make_unique<infrastructure::AdimecCamera>(std::move(camera_protocol), std::move(lens_protocol));
    ASSERT_NE(nullptr, camera);
}

TEST_F(AdimecCameraTests, ConnectDisconnect) {
    const auto connect_result = camera_->open();
    ASSERT_TRUE(connect_result.isSuccess());

    const auto disconnect_result = camera_->close();
    ASSERT_TRUE(disconnect_result.isSuccess());
}

TEST_F(AdimecCameraTests, CanBeConnected) {
    const auto result = camera_->open();
    ASSERT_TRUE(result.isSuccess());
}

TEST_F(AdimecCameraTests, ErrorOnDisconnectWhenNotConnected) {
    const auto result = camera_->close();
    ASSERT_TRUE(result.isError());
}

TEST_F(AdimecCameraTests, ZoomOperations) {
    const auto result = camera_->open();
    ASSERT_TRUE(result.isSuccess());

    constexpr auto expected_value = 2u;

    const auto set_result = camera_->setZoom(expected_value);
    ASSERT_TRUE(set_result.isSuccess());

    const auto get_result = camera_->getZoom();
    ASSERT_TRUE(get_result.isSuccess());
    EXPECT_EQ(expected_value, get_result.value());
}

TEST_F(AdimecCameraTests, FocusOperations) {
    const auto result = camera_->open();
    ASSERT_TRUE(result.isSuccess());

    constexpr auto expected_value = 2u;

    const auto set_result = camera_->setFocus(expected_value);
    ASSERT_TRUE(set_result.isSuccess());

    const auto get_result = camera_->getFocus();
    ASSERT_TRUE(get_result.isSuccess());
    EXPECT_EQ(expected_value, get_result.value());
}