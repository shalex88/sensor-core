#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <memory>
#include <thread>

#include "app/Application.h"
#include "common/config/ConfigManager.h"
#include "../../utils/GrpcClient.h"

using namespace service;
using namespace testing;

class ServiceSystemTests : public Test {
protected:
    void SetUp() override {
        const char* config_path = "../../config/config-simulator.yaml";
        char* argv[] = {const_cast<char*>("camera-service"), const_cast<char*>("-c"), const_cast<char*>(config_path)};
        const int argc = 3;

        app = std::make_unique<app::Application>(argc, argv);
        ASSERT_NE(nullptr, app);

        const auto init_result = app->initialize();
        ASSERT_TRUE(init_result.isSuccess()) << "Initialization failed: " << init_result.error();

        const auto start_result = app->start();
        ASSERT_TRUE(start_result.isSuccess()) << "Failed to start: " << start_result.error();

        std::this_thread::sleep_for(std::chrono::seconds(1));

        config = std::make_unique<common::ConfigManager>(config_path);
        const auto& api_config_obj = config->getApiConfig();
        server_address = api_config_obj.server_address;
    }

    void TearDown() override {
        if (app) {
            const auto stop_result = app->stop();
            EXPECT_TRUE(stop_result.isSuccess()) << "Shutdown error: " << stop_result.error();
        }
    }

    std::unique_ptr<app::Application> app;
    std::unique_ptr<common::ConfigManager> config;
    std::string server_address;
};

TEST_F(ServiceSystemTests, CameraRequestResponse) {
    std::cout << "Connecting to server at " << server_address << "\n";
    const auto channel = CreateChannel(server_address, grpc::InsecureChannelCredentials());
    const GrpcClient client(channel);

    constexpr common::types::zoom test_zoom = 1u;
    std::cout << "Test SetZoom " << test_zoom <<" and GetZoom" << "\n";
    ASSERT_TRUE(client.setZoom(test_zoom).isSuccess());

    const auto zoom_result = client.getZoom();
    ASSERT_TRUE(zoom_result.isSuccess());
    EXPECT_EQ(test_zoom, zoom_result.value());

    constexpr common::types::focus test_focus = 1u;
    std::cout << "Test SetFocus " << test_focus <<" and GetFocus" << "\n";
    ASSERT_TRUE(client.setFocus(test_focus).isError());

    std::cout << "Disable Autofocus" << "\n";
    ASSERT_TRUE(client.setAutoFocus(false).isSuccess());

    std::cout << "Test SetFocus " << test_focus <<" and GetFocus" << "\n";
    ASSERT_TRUE(client.setFocus(test_focus).isSuccess());

    const auto focus_result = client.getFocus();
    ASSERT_TRUE(focus_result.isSuccess());
    EXPECT_EQ(test_focus, focus_result.value());
}
