#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "../../../src/infrastructure/camera/CameraFactory.h"
#include "common/config/ConfigManager.h"
#include "infrastructure/camera/hal/ICamera.h"

using namespace service;
using namespace testing;

TEST(CameraFactoryTests, CreateSonyCameraSuccess) {
    common::InfrastructureConfig config;
    config.camera = "sony";
    common::EndpointConfig endpoint;
    endpoint.address = "fake";
    endpoint.configuration.emplace("baud_rate", "9600");
    config.endpoints.push_back(endpoint);

    const auto camera = infrastructure::CameraFactory::createCamera(config);
    ASSERT_NE(nullptr, camera);
    ASSERT_TRUE(camera.get() != nullptr);
}

TEST(CameraFactoryTests, ThrowsOnUnknownType) {
    common::InfrastructureConfig config;
    config.camera = "invalid_camera";
    common::EndpointConfig endpoint;
    endpoint.address = "fake";
    endpoint.configuration.emplace("baud_rate", "9600");
    config.endpoints.push_back(endpoint);

    EXPECT_THROW(infrastructure::CameraFactory::createCamera(config), std::invalid_argument);
}

TEST(CameraFactoryTests, ThrowsOnEmptyType) {
    common::InfrastructureConfig config;
    config.camera = "";
    common::EndpointConfig endpoint;
    endpoint.address = "fake";
    endpoint.configuration.emplace("baud_rate", "9600");
    config.endpoints.push_back(endpoint);

    EXPECT_THROW(infrastructure::CameraFactory::createCamera(config), std::invalid_argument);
}
