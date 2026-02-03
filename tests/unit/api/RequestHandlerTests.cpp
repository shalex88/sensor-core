#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "api/RequestHandler.h"
#include "common/types/Result.h"
#include "common/types/CameraCapabilities.h"
#include "../../Mocks.h"

class RequestHandlerTests : public Test {
protected:
    RequestHandlerTests() {
        core = new NiceMock<CoreMock>();
        auto core_obj = std::unique_ptr<core::ICore>(core);
        request_handler = std::make_unique<api::RequestHandler>(std::move(core_obj));
    }
    std::unique_ptr<api::RequestHandler> request_handler;
    NiceMock<CoreMock>* core {};
};

TEST_F(RequestHandlerTests, CreationSuccess) {
    ASSERT_NE(nullptr, request_handler);
}

TEST_F(RequestHandlerTests, CreationFailNoCore) {
    EXPECT_THROW(api::RequestHandler request_handler(nullptr), std::invalid_argument);
}

TEST_F(RequestHandlerTests, StartSuccess) {
    const auto result = request_handler->start();
    ASSERT_TRUE(result.isSuccess());
}

TEST_F(RequestHandlerTests, StartFailOnInitialize) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::error("Initialize failed")));

    const auto result = request_handler->start();
    ASSERT_TRUE(result.isError());
}

TEST_F(RequestHandlerTests, StopSuccessIfRunning) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, stop())
        .WillOnce(Return(Result<void>::success()));

    const auto start_result = request_handler->start();
    ASSERT_TRUE(start_result.isSuccess());

    const auto stop_result = request_handler->stop();
    ASSERT_TRUE(stop_result.isSuccess());
}

TEST_F(RequestHandlerTests, StopSuccessIfNotRunning) {
    const auto result = request_handler->stop();
    ASSERT_TRUE(result.isSuccess());
}

TEST_F(RequestHandlerTests, StopFailsIfCoreShutdownFails) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, stop())
        .WillOnce(Return(Result<void>::error("Shutdown failed")));

    const auto start_result = request_handler->start();
    ASSERT_TRUE(start_result.isSuccess());

    const auto stop_result = request_handler->stop();
    ASSERT_TRUE(stop_result.isError());
}

TEST_F(RequestHandlerTests, ZoomOperations) {
    Sequence s;
    EXPECT_CALL(*core, start())
        .InSequence(s)
        .WillOnce(Return(Result<void>::success()));

    EXPECT_CALL(*core, setZoom(0, 2))
        .InSequence(s)
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, getZoom(0))
        .InSequence(s)
        .WillOnce(Return(Result<common::types::zoom>::success(2u)));
    EXPECT_CALL(*core, stop())
        .InSequence(s)
        .WillOnce(Return(Result<void>::success()));

    const auto start_result = request_handler->start();
    ASSERT_TRUE(start_result.isSuccess()) << "Failed to start: " << start_result.error();

    const auto set_result = request_handler->setZoom(0, 2);
    ASSERT_TRUE(set_result.isSuccess()) << "Failed to set zoom: " << set_result.error();

    const auto get_result = request_handler->getZoom(0);
    ASSERT_TRUE(get_result.isSuccess()) << "Failed to get zoom: " << get_result.error();
    EXPECT_EQ(2, get_result.value());

    const auto stop_result = request_handler->stop();
    ASSERT_TRUE(stop_result.isSuccess()) << "Failed to stop: " << stop_result.error();
}

TEST_F(RequestHandlerTests, ZoomOperationsFailIfNotRunning) {
    const auto set_result = request_handler->setZoom(0, 2);
    ASSERT_TRUE(set_result.isError());

    const auto get_result = request_handler->getZoom(0);
    ASSERT_TRUE(get_result.isError());
}

TEST_F(RequestHandlerTests, FocusOperations) {
    Sequence s;
    EXPECT_CALL(*core, start())
        .InSequence(s)
        .WillOnce(Return(Result<void>::success()));

    EXPECT_CALL(*core, setFocus(0, 1))
        .InSequence(s)
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, getFocus(0))
        .InSequence(s)
        .WillOnce(Return(Result<common::types::focus>::success(1u)));
    EXPECT_CALL(*core, stop())
        .InSequence(s)
        .WillOnce(Return(Result<void>::success()));

    const auto start_result = request_handler->start();
    ASSERT_TRUE(start_result.isSuccess()) << "Failed to start: " << start_result.error();

    const auto set_result = request_handler->setFocus(0, 1);
    ASSERT_TRUE(set_result.isSuccess()) << "Failed to set focus: " << set_result.error();

    const auto get_result = request_handler->getFocus(0);
    ASSERT_TRUE(get_result.isSuccess()) << "Failed to get focus: " << get_result.error();
    EXPECT_EQ(1, get_result.value());

    const auto stop_result = request_handler->stop();
    ASSERT_TRUE(stop_result.isSuccess()) << "Failed to stop: " << stop_result.error();
}

TEST_F(RequestHandlerTests, FocusOperationsFailIfNotRunning) {
    const auto set_result = request_handler->setFocus(0, 2);
    ASSERT_TRUE(set_result.isError());

    const auto get_result = request_handler->getFocus(0);
    ASSERT_TRUE(get_result.isError());
}

TEST_F(RequestHandlerTests, GetInfoSuccess) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, getInfo(0))
        .WillOnce(Return(Result<common::types::info>::success(std::string("Camera Info"))));
    EXPECT_CALL(*core, stop())
        .WillOnce(Return(Result<void>::success()));

    ASSERT_TRUE(request_handler->start().isSuccess());

    const auto info_result = request_handler->getInfo(0);
    ASSERT_TRUE(info_result.isSuccess());
    EXPECT_EQ(info_result.value(), "Camera Info");

    ASSERT_TRUE(request_handler->stop().isSuccess());
}

TEST_F(RequestHandlerTests, GetInfoFailsIfNotRunning) {
    const auto result = request_handler->getInfo(0);
    ASSERT_TRUE(result.isError());
}

TEST_F(RequestHandlerTests, EnableAutoFocusSuccess) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, enableAutoFocus(0, true))
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, stop())
        .WillOnce(Return(Result<void>::success()));

    ASSERT_TRUE(request_handler->start().isSuccess());
    ASSERT_TRUE(request_handler->enableAutoFocus(0, true).isSuccess());
    ASSERT_TRUE(request_handler->stop().isSuccess());
}

TEST_F(RequestHandlerTests, EnableAutoFocusFailsIfNotRunning) {
    const auto result = request_handler->enableAutoFocus(0, true);
    ASSERT_TRUE(result.isError());
}

TEST_F(RequestHandlerTests, GoToMinZoomSuccess) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, goToMinZoom(0))
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, stop())
        .WillOnce(Return(Result<void>::success()));

    ASSERT_TRUE(request_handler->start().isSuccess());
    ASSERT_TRUE(request_handler->goToMinZoom(0).isSuccess());
    ASSERT_TRUE(request_handler->stop().isSuccess());
}

TEST_F(RequestHandlerTests, GoToMinZoomFailsIfNotRunning) {
    const auto result = request_handler->goToMinZoom(0);
    ASSERT_TRUE(result.isError());
}

TEST_F(RequestHandlerTests, GoToMaxZoomSuccess) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, goToMaxZoom(0))
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, stop())
        .WillOnce(Return(Result<void>::success()));

    ASSERT_TRUE(request_handler->start().isSuccess());
    ASSERT_TRUE(request_handler->goToMaxZoom(0).isSuccess());
    ASSERT_TRUE(request_handler->stop().isSuccess());
}

TEST_F(RequestHandlerTests, GoToMaxZoomFailsIfNotRunning) {
    const auto result = request_handler->goToMaxZoom(0);
    ASSERT_TRUE(result.isError());
}

TEST_F(RequestHandlerTests, StabilizeSuccess) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, stabilize(0, true))
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, stop())
        .WillOnce(Return(Result<void>::success()));

    ASSERT_TRUE(request_handler->start().isSuccess());
    ASSERT_TRUE(request_handler->stabilize(0, true).isSuccess());
    ASSERT_TRUE(request_handler->stop().isSuccess());
}

TEST_F(RequestHandlerTests, StabilizeFailsIfNotRunning) {
    const auto result = request_handler->stabilize(0, true);
    ASSERT_TRUE(result.isError());
}

TEST_F(RequestHandlerTests, GetCapabilitiesSuccess) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));

    const common::capabilities::CapabilityList expected {
        common::capabilities::Capability::Zoom,
        common::capabilities::Capability::Focus,
        common::capabilities::Capability::Stabilization};

    EXPECT_CALL(*core, getCapabilities(0))
        .WillOnce(Return(Result<common::capabilities::CapabilityList>::success(expected)));

    EXPECT_CALL(*core, stop())
        .WillOnce(Return(Result<void>::success()));

    ASSERT_TRUE(request_handler->start().isSuccess());

    const auto result = request_handler->getCapabilities(0);
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), expected);

    ASSERT_TRUE(request_handler->stop().isSuccess());
}

TEST_F(RequestHandlerTests, GetCapabilitiesFailsIfNotRunning) {
    const auto result = request_handler->getCapabilities(0);
    ASSERT_TRUE(result.isError());
}

TEST_F(RequestHandlerTests, IsRunningInitiallyFalse) {
    EXPECT_FALSE(request_handler->isRunning());
}

TEST_F(RequestHandlerTests, IsRunningTrueAfterStart) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));

    ASSERT_TRUE(request_handler->start().isSuccess());
    EXPECT_TRUE(request_handler->isRunning());
}

TEST_F(RequestHandlerTests, IsRunningFalseAfterStop) {
    EXPECT_CALL(*core, start())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*core, stop())
        .WillOnce(Return(Result<void>::success()));

    ASSERT_TRUE(request_handler->start().isSuccess());
    EXPECT_TRUE(request_handler->isRunning());

    ASSERT_TRUE(request_handler->stop().isSuccess());
    EXPECT_FALSE(request_handler->isRunning());
}
