#include <chrono>
#include <thread>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
/* Add your project include files here */
#include "../../Mocks.h"
#include "api/RequestHandler.h"
#include "api/grpc/GrpcTransport.h"
#include "common/types/Result.h"

class GrpcTransportTests : public Test {
protected:
    void SetUp() override {
        request_handler = std::make_unique<api::RequestHandler>(std::make_unique<CoreMock>());
        grpc_transport = std::make_unique<api::GrpcTransport>(*request_handler);
    }

    std::unique_ptr<api::RequestHandler> request_handler;
    std::unique_ptr<api::GrpcTransport> grpc_transport;
    std::string server = "0.0.0.0";
    uint16_t port = 50051;
};

TEST_F(GrpcTransportTests, CreationSuccess) {
    ASSERT_NE(nullptr, grpc_transport);
}

TEST_F(GrpcTransportTests, StartServerSuccess) {
    const auto result = grpc_transport->start(server, port);
    ASSERT_TRUE(result.isSuccess());
    EXPECT_TRUE(grpc_transport->stop().isSuccess());
}

TEST_F(GrpcTransportTests, StartServerOnInvalidPortShouldFail) {
    const auto result = grpc_transport->start("", 0);
    ASSERT_TRUE(result.isError());
}

TEST_F(GrpcTransportTests, StopServerWhenNotStartedShouldSucceed) {
    const auto result = grpc_transport->stop();
    ASSERT_TRUE(result.isSuccess());
}

TEST_F(GrpcTransportTests, StopRunningServerShouldSucceed) {
    // Start server first
    const auto start_result = grpc_transport->start(server, port);
    ASSERT_TRUE(start_result.isSuccess());

    // Then stop it
    const auto stop_result = grpc_transport->stop();
    ASSERT_TRUE(stop_result.isSuccess());
}

TEST_F(GrpcTransportTests, StopServerMultipleTimesShouldSucceed) {
    // Start and stop once
    EXPECT_TRUE(grpc_transport->start(server, port).isSuccess());
    const auto first_stop = grpc_transport->stop();
    ASSERT_TRUE(first_stop.isSuccess());

    // Stop again when already stopped
    const auto second_stop = grpc_transport->stop();
    ASSERT_TRUE(second_stop.isSuccess());
}

TEST_F(GrpcTransportTests, RunLoopWithoutStartShouldFail) {
    const auto result = grpc_transport->runLoop();
    ASSERT_TRUE(result.isError());
}

TEST_F(GrpcTransportTests, RunLoopAfterStopShouldFail) {
    // Start and stop the server
    EXPECT_TRUE(grpc_transport->start(server, port).isSuccess());
    EXPECT_TRUE(grpc_transport->stop().isSuccess());

    // Try to run loop after stop
    const auto result = grpc_transport->runLoop();
    ASSERT_TRUE(result.isError());
}

TEST_F(GrpcTransportTests, RunLoopWithRunningServerShouldSucceed) {
    EXPECT_TRUE(grpc_transport->start(server, port).isSuccess());

    std::jthread server_thread([&] {
        const auto result = grpc_transport->runLoop();
        ASSERT_TRUE(result.isSuccess());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(grpc_transport->stop().isSuccess());
}

TEST_F(GrpcTransportTests, StartServerTwiceWithoutStopFails) {
    EXPECT_TRUE(grpc_transport->start(server, port).isSuccess());

    // Second start without stopping should fail
    const auto second_start = grpc_transport->start(server, port);
    ASSERT_TRUE(second_start.isError());

    // Cleanup
    EXPECT_TRUE(grpc_transport->stop().isSuccess());
}

TEST_F(GrpcTransportTests, StartWithDifferentPortsSucceeds) {
    // Start on first port
    EXPECT_TRUE(grpc_transport->start(server, 50052).isSuccess());
    EXPECT_TRUE(grpc_transport->stop().isSuccess());

    // Start on second port
    EXPECT_TRUE(grpc_transport->start(server, 50053).isSuccess());
    EXPECT_TRUE(grpc_transport->stop().isSuccess());
}

TEST_F(GrpcTransportTests, StartWithEmptyAddressFails) {
    const auto result = grpc_transport->start("", port);
    ASSERT_TRUE(result.isError());
}

TEST_F(GrpcTransportTests, StartWithInvalidFormatFails) {
    const auto result = grpc_transport->start(server, 0);
    ASSERT_TRUE(result.isError());
}

TEST_F(GrpcTransportTests, RunLoopBeforeStartFails) {
    const auto result = grpc_transport->runLoop();
    ASSERT_TRUE(result.isError());
}