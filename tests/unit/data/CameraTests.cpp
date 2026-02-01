#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "../../Mocks.h"
#include "common/types/Result.h"
#include "common/types/CameraCapabilities.h"
#include "infrastructure/camera/hal/ICamera.h"
#include "infrastructure/camera/hal/Camera.h"
#include "infrastructure/camera/devices/FakeAdvancedCamera.h"

using namespace service;
using namespace testing;

class CameraTests : public Test {
protected:
    CameraTests() {
        auto camera_strategy_obj = std::make_unique<NiceMock<MockCameraHw>>();
        camera_hw_ = camera_strategy_obj.get();
        camera_ = std::make_unique<infrastructure::Camera>(std::move(camera_strategy_obj));
    }

    NiceMock<MockCameraHw>* camera_hw_ {};
    std::unique_ptr<infrastructure::ICamera> camera_;
};

TEST_F(CameraTests, CanBeConstructed) {
    const auto camera = std::make_unique<infrastructure::FakeAdvancedCamera>();
    ASSERT_NE(nullptr, camera);
}

TEST_F(CameraTests, InitiallyNotConnected) {
    EXPECT_FALSE(camera_->isConnected());
}

TEST_F(CameraTests, ConnectDisconnect) {
    const auto connect_result = camera_->open();
    ASSERT_TRUE(connect_result.isSuccess());
    ASSERT_TRUE(camera_->isConnected());

    const auto disconnect_result = camera_->close();
    ASSERT_TRUE(disconnect_result.isSuccess());
    EXPECT_FALSE(camera_->isConnected());
}

TEST_F(CameraTests, ConnectFailsWhenCameraCantConnect) {
    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::error("error")));

    const auto connect_result = camera_->open();
    ASSERT_TRUE(connect_result.isError());
    ASSERT_FALSE(camera_->isConnected());
}

TEST_F(CameraTests, ReconnectWhenAlreadyConnectedFails) {
    const auto connect_result = camera_->open();
    ASSERT_TRUE(connect_result.isSuccess());

    const auto second_connect_result = camera_->open();
    ASSERT_TRUE(second_connect_result.isError());
    ASSERT_TRUE(second_connect_result.error().find("connected") != std::string::npos);
}

TEST_F(CameraTests, DisconnectWhenNotConnectedSucceeds) {
    EXPECT_TRUE(camera_->close().isSuccess());
    ASSERT_FALSE(camera_->isConnected());
}

TEST_F(CameraTests, DisonnectWhenConnectedSucceeds) {
    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));

    EXPECT_TRUE(camera_->open().isSuccess());
    ASSERT_TRUE(camera_->close().isSuccess());
}

TEST_F(CameraTests, SetZoomWhenNotConnectedFail) {
    const auto result = camera_->setZoom(2);
    ASSERT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Camera not connected");
}

TEST_F(CameraTests, GetZoomWhenNotConnectedFail) {
    const auto result = camera_->getZoom();
    ASSERT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Camera not connected");
}

TEST_F(CameraTests, SetFocusWhenNotConnectedFail) {
    const auto result = camera_->setFocus(1);
    ASSERT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Camera not connected");
}

TEST_F(CameraTests, GetFocusWhenNotConnectedFail) {
    const auto result = camera_->getFocus();
    ASSERT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Camera not connected");
}

TEST_F(CameraTests, GetCapabilitiesWhenNotConnectedFail) {
    const auto result = camera_->getCapabilities();
    ASSERT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Camera not connected");
}

TEST_F(CameraTests, SetValidZoomSuccess) {
    constexpr auto normalized_zoom = 50;

    constexpr common::types::ZoomRange zoom_limits{.min = 0, .max = 100};
    EXPECT_CALL(*camera_hw_, getZoomLimits())
        .WillRepeatedly(Return(zoom_limits));

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));

    constexpr auto expected_hw_value = 50;
    EXPECT_CALL(*camera_hw_, setZoom(expected_hw_value))
        .WillOnce(Return(Result<void>::success()));

    const auto connect_result = camera_->open();
    ASSERT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->setZoom(normalized_zoom);
    ASSERT_TRUE(set_result.isSuccess());
}

TEST_F(CameraTests, SetInvalidZoomFail) {
    constexpr auto expected_value = -2;

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));

    const auto connect_result = camera_->open();
    EXPECT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->setZoom(expected_value);
    ASSERT_TRUE(set_result.isError());
}

TEST_F(CameraTests, SetValidZoomWhenCameraErrorFails) {
    constexpr auto normalized_zoom = 2;

    constexpr common::types::ZoomRange zoom_limits{.min = 0, .max = 1000};
    EXPECT_CALL(*camera_hw_, getZoomLimits())
        .WillRepeatedly(Return(zoom_limits));

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));

    constexpr auto expected_hw_value = 20;
    EXPECT_CALL(*camera_hw_, setZoom(expected_hw_value))
        .WillOnce(Return(Result<void>::error("error")));

    const auto connect_result = camera_->open();
    EXPECT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->setZoom(normalized_zoom);
    ASSERT_TRUE(set_result.isError());
}

TEST_F(CameraTests, GetValidZoomWhenCameraErrorFails) {
    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera_hw_, getZoom())
        .WillOnce(Return(Result<common::types::zoom>::error("error")));

    const auto connect_result = camera_->open();
    EXPECT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->getZoom();
    ASSERT_TRUE(set_result.isError());
}

TEST_F(CameraTests, GetValidZoomSuccess) {
    constexpr auto hardware_zoom_value = 200u;

    constexpr common::types::ZoomRange zoom_limits{.min = 0, .max = 1000};
    EXPECT_CALL(*camera_hw_, getZoomLimits())
        .WillRepeatedly(Return(zoom_limits));

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera_hw_, getZoom())
        .WillOnce(Return(Result<common::types::zoom>::success(hardware_zoom_value)));

    const auto connect_result = camera_->open();
    EXPECT_TRUE(connect_result.isSuccess());

    const auto get_result = camera_->getZoom();
    EXPECT_TRUE(get_result.isSuccess());

    constexpr auto expected_normalized_value = 20u;
    ASSERT_EQ(get_result.value<common::types::zoom>(), expected_normalized_value);
}

TEST_F(CameraTests, GetInvalidZoomFail) {
    constexpr auto expected_value = -2u;

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera_hw_, getZoom())
        .WillOnce(Return(Result<common::types::zoom>::success(expected_value)));

    const auto connect_result = camera_->open();
    EXPECT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->getZoom();
    ASSERT_TRUE(set_result.isError());
}

TEST_F(CameraTests, SetValidFocusSuccess) {
    constexpr auto normalized_focus = 2u;

    constexpr common::types::FocusRange focus_limits{.min = 0, .max = 1000};
    EXPECT_CALL(*camera_hw_, getFocusLimits())
        .WillRepeatedly(Return(focus_limits));

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));

    constexpr auto expected_hw_value = 20u;
    EXPECT_CALL(*camera_hw_, setFocus(expected_hw_value))
        .WillOnce(Return(Result<void>::success()));

    const auto connect_result = camera_->open();
    ASSERT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->setFocus(normalized_focus);
    ASSERT_TRUE(set_result.isSuccess());
}

TEST_F(CameraTests, GetValidFocusSuccess) {
    constexpr auto hardware_focus_value = 200u;

    constexpr common::types::FocusRange focus_limits{.min = 0, .max = 1000};
    EXPECT_CALL(*camera_hw_, getFocusLimits())
        .WillRepeatedly(Return(focus_limits));

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera_hw_, getFocus())
        .WillOnce(Return(Result<common::types::focus>::success(hardware_focus_value)));

    const auto connect_result = camera_->open();
    EXPECT_TRUE(connect_result.isSuccess());

    const auto get_result = camera_->getFocus();
    EXPECT_TRUE(get_result.isSuccess());

    constexpr auto expected_normalized_value = 20u;
    ASSERT_EQ(get_result.value<common::types::focus>(), expected_normalized_value);
}

TEST_F(CameraTests, SetValidFocusWhenCameraErrorFails) {
    constexpr auto normalized_focus = 2u;

    constexpr common::types::FocusRange focus_limits{.min = 0, .max = 1000};
    EXPECT_CALL(*camera_hw_, getFocusLimits())
        .WillRepeatedly(Return(focus_limits));

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));

    constexpr auto expected_hw_value = 20u;
    EXPECT_CALL(*camera_hw_, setFocus(expected_hw_value))
        .WillOnce(Return(Result<void>::error("error")));

    const auto connect_result = camera_->open();
    EXPECT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->setFocus(normalized_focus);
    ASSERT_TRUE(set_result.isError());
}

TEST_F(CameraTests, GetCapabilitiesReturnsAllSupported) {
    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));

    ASSERT_TRUE(camera_->open().isSuccess());

    const auto result = camera_->getCapabilities();
    ASSERT_TRUE(result.isSuccess());

    const auto& capabilities = result.value();
    EXPECT_THAT(capabilities, UnorderedElementsAre(
        common::capabilities::Capability::Zoom,
        common::capabilities::Capability::Focus,
        common::capabilities::Capability::AutoFocus,
        common::capabilities::Capability::Info,
        common::capabilities::Capability::Stabilization));
}

TEST_F(CameraTests, GetValidFocusWhenCameraErrorFails) {
    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera_hw_, getFocus())
        .WillOnce(Return(Result<common::types::focus>::error("error")));

    const auto connect_result = camera_->open();
    EXPECT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->getFocus();
    ASSERT_TRUE(set_result.isError());
}

TEST_F(CameraTests, SetInvalidFocusFail) {
    constexpr auto expected_value = -2u;

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));

    const auto connect_result = camera_->open();
    ASSERT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->setFocus(expected_value);
    ASSERT_TRUE(set_result.isError());
}


TEST_F(CameraTests, GetInvalidFocusFail) {
    constexpr auto expected_value = -2u;

    EXPECT_CALL(*camera_hw_, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera_hw_, getFocus())
        .WillOnce(Return(Result<common::types::focus>::success(expected_value)));

    const auto connect_result = camera_->open();
    EXPECT_TRUE(connect_result.isSuccess());

    const auto set_result = camera_->getFocus();
    ASSERT_TRUE(set_result.isError());
}
