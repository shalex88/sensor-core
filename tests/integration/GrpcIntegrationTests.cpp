#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include <chrono>
#include <memory>
#include <thread>

#include "api/GrpcTransport.h"
#include "api/RequestHandler.h"
#include "common/logger/Logger.h"
#include "common/types/CameraCapabilities.h"
#include "../../utils/GrpcClient.h"
#include "../Mocks.h"

class GrpcIntegrationTests : public Test {
protected:
    void SetUp() override {
        CONFIGURE_LOGGER("grpc-integration-tests", "debug");
        auto core_obj = std::make_unique<CoreMock>();
        core = core_obj.get();

        EXPECT_CALL(*core, start())
            .WillOnce(Return(Result<void>::success()));

        request_handler = std::make_unique<api::RequestHandler>(std::move(core_obj));
        grpc_transport = std::make_unique<api::GrpcTransport>(*request_handler);

        ASSERT_TRUE(request_handler->start().isSuccess());
        ASSERT_TRUE(grpc_transport->start(server_address).isSuccess());

        // Run the server loop in a separate thread
        server_thread = std::jthread([this] {
            server_result = grpc_transport->runLoop();
        });

        // Give the server a moment to start listening
        std::cout << "Connecting to server at " << server_address << "\n";
        const auto channel = CreateChannel(server_address, grpc::InsecureChannelCredentials());
        client = std::make_unique<GrpcClient>(channel);
    }

    void TearDown() override {
        EXPECT_CALL(*core, stop())
            .WillOnce(Return(Result<void>::success()));

        if (grpc_transport) {
            ASSERT_TRUE(grpc_transport->stop().isSuccess());
        }
    }

    CoreMock* core {}; // Raw pointer to access the mock
    std::unique_ptr<api::RequestHandler> request_handler;
    std::unique_ptr<api::GrpcTransport> grpc_transport;
    std::string server_address = "0.0.0.0:50051";
    std::unique_ptr<GrpcClient> client;
    std::jthread server_thread;
    Result<void> server_result;
};

TEST_F(GrpcIntegrationTests, SetZoomAndGetZoomSuccess) {
    constexpr uint32_t test_zoom = 3u;

    EXPECT_CALL(*core, setZoom(test_zoom))
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, getZoom())
        .WillOnce(Return(Result<common::types::zoom>::success(test_zoom)));

    std::cout << "Test SetZoom " << test_zoom << " getZoom" << "\n";
    ASSERT_TRUE(client->setZoom(test_zoom).isSuccess());
    const auto get_zoom_result = client->getZoom();
    ASSERT_TRUE(get_zoom_result.isSuccess());
    EXPECT_EQ(get_zoom_result.value(), test_zoom);
}

TEST_F(GrpcIntegrationTests, RequestFailOnCoreFail) {
    constexpr uint32_t test_zoom = 3u;

    EXPECT_CALL(*core, setZoom(test_zoom))
        .WillOnce(Return(Result<void>::error("Fail")));

    ASSERT_TRUE(client->setZoom(test_zoom).isError());
}

TEST_F(GrpcIntegrationTests, RequestFailOnTimeout) {
    constexpr uint32_t test_zoom = 3u;

    EXPECT_CALL(*core, setZoom(test_zoom))
        .WillOnce(Invoke([] {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return Result<void>::success();
        }));

    const auto result = client->setZoom(test_zoom);
    ASSERT_TRUE(result.isError());
    ASSERT_TRUE(result.error().find("Deadline") != std::string::npos);
}

TEST_F(GrpcIntegrationTests, GetCapabilitiesSuccess) {
    const common::capabilities::CapabilityList expected {
        common::capabilities::Capability::Zoom,
        common::capabilities::Capability::Focus};

    EXPECT_CALL(*core, getCapabilities())
        .WillOnce(Return(Result<common::capabilities::CapabilityList>::success(expected)));

    const auto result = client->getCapabilities();
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), expected);
}
