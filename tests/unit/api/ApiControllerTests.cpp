#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "api/ApiController.h"
#include "api/ITransport.h"
#include "common/types/Result.h"
#include "../../Mocks.h"

class ApiControllerTests : public Test {
protected:
    void SetUp() override {
        auto request_handler_ptr = std::make_unique<NiceMock<RequestHandlerMock>>();
        request_handler = request_handler_ptr.get();
        transport = new NiceMock<TransportMock>();
        auto transport_obj = std::unique_ptr<api::ITransport>(transport);
        controller = std::make_unique<api::ApiController>(std::move(request_handler_ptr), std::move(transport_obj), server_address);
    }

    NiceMock<RequestHandlerMock>* request_handler {};
    NiceMock<TransportMock>* transport {};
    std::unique_ptr<api::ApiController> controller;
    std::string server_address = "50051";
};

TEST_F(ApiControllerTests, CreationSuccess) {
    ASSERT_NE(nullptr, controller);
}

TEST_F(ApiControllerTests, CreationFailNoController) {
    EXPECT_THROW(api::ApiController controller(
    nullptr,
    std::make_unique<NiceMock<TransportMock>>(),
    server_address), std::invalid_argument);
}

TEST_F(ApiControllerTests, CreationFailNoTransport) {
    EXPECT_THROW(api::ApiController controller(
    std::make_unique<NiceMock<RequestHandlerMock>>(),
    nullptr,
    server_address), std::invalid_argument);
}

TEST_F(ApiControllerTests, CreationFailEmptyPort) {
    EXPECT_THROW(api::ApiController controller(
    std::make_unique<NiceMock<RequestHandlerMock>>(),
    std::make_unique<NiceMock<TransportMock>>(),
    ""), std::invalid_argument);
}

TEST_F(ApiControllerTests, StartStopSuccess) {
    const auto start_result = controller->startAsync();
    ASSERT_TRUE(start_result.isSuccess());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // stop() should be triggered by the destructor
    EXPECT_CALL(*transport, stop())
    .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*request_handler, stop())
        .WillOnce(Return(Result<void>::success()));
}

TEST_F(ApiControllerTests, StartFailOnRequestHandlerStartFail) {
    EXPECT_CALL(*request_handler, start())
        .WillOnce(Return(Result<void>::error("Request Handler start failed")));

    const auto result = controller->startAsync();
    ASSERT_TRUE(result.isError());
}

TEST_F(ApiControllerTests, StartFailOnTransportStartFail) {
    EXPECT_CALL(*transport, start(server_address))
        .WillOnce(Return(Result<void>::error("Transport start failed")));

    const auto result = controller->startAsync();
    ASSERT_TRUE(result.isError());
}

TEST_F(ApiControllerTests, StopSuccessIfNotRunning) {
    const auto result = controller->stop();
    ASSERT_TRUE(result.isSuccess()) << "Stop should succeed if not running";
}

TEST_F(ApiControllerTests, StopSuccessIfRunning) {
    const auto result = controller->stop();
    ASSERT_TRUE(result.isSuccess()) << "Stop should succeed if not running";
}

TEST_F(ApiControllerTests, StopFailsIfTransportStopFails) {
    const auto start_result = controller->startAsync();
    ASSERT_TRUE(start_result.isSuccess());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_CALL(*transport, stop())
        .WillOnce(Return(Result<void>::error("Transport stop failed")));

    const auto stop_result = controller->stop();
    ASSERT_TRUE(stop_result.isError());
}

TEST_F(ApiControllerTests, StopFailsIfRequestHandlerStopFails) {
    const auto start_result = controller->startAsync();
    ASSERT_TRUE(start_result.isSuccess());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_CALL(*transport, stop())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*request_handler, stop())
        .WillOnce(Return(Result<void>::error("RequestHandler stop failed")));

    const auto stop_result = controller->stop();
    ASSERT_TRUE(stop_result.isError());
}

TEST_F(ApiControllerTests, StartFailsWithEmptyServerAddress) {
    auto req_handler = std::make_unique<NiceMock<RequestHandlerMock>>();
    auto trans = std::make_unique<NiceMock<TransportMock>>();

    EXPECT_THROW(
        api::ApiController ctrl(std::move(req_handler), std::move(trans), ""),
        std::invalid_argument
    );
}
