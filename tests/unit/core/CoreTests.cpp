#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "core/Core.h"
#include "../../Mocks.h"
#include "common/types/Result.h"
#include "common/types/CameraCapabilities.h"

class CoreTests : public Test {
protected:
    CoreTests() {
        camera = std::make_unique<MockCameraHal>();
    }
    std::unique_ptr<MockCameraHal> camera;
};

TEST_F(CoreTests, CanBeCreated) {
    EXPECT_NO_THROW(core::Core(std::move(camera)));
}

TEST_F(CoreTests, ThrowsOnNullCamera) {
    EXPECT_THROW(core::Core(nullptr), std::invalid_argument);
}

TEST_F(CoreTests, InitializeSuccessWhenDisconnected) {
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(true)); // for shutdown
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::success()));

    core::Core core(std::move(camera));
    const auto result = core.start();
    ASSERT_TRUE(result.isSuccess()) << "Failed to initialize: " << result.error();
}

TEST_F(CoreTests, InitializeSuccessWhenAlreadyConnected) {
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(true))
        .WillOnce(Return(true)); // for shutdown
    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::success()));

    core::Core core(std::move(camera));
    const auto result = core.start();
    ASSERT_TRUE(result.isSuccess()) << "Failed to initialize: " << result.error();
}

TEST_F(CoreTests, InitializeFailsOnConnectError) {
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false));
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::error("Failed to open")));

    core::Core core(std::move(camera));
    const auto result = core.start();
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("Failed to open"));
}

TEST_F(CoreTests, ZoomOperationsSuccess) {
    // Set up all expectations before moving the camera
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(true)); // for shutdown
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, setZoom(2))
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, getZoom())
        .WillOnce(Return(Result<common::types::zoom>::success(2u)));
    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::success()));


    // Now create the core with the moved camera
    core::Core core(std::move(camera));
    const auto init_result = core.start();
    ASSERT_TRUE(init_result.isSuccess()) << "Failed to initialize: " << init_result.error();

    const auto set_result = core.setZoom(2);
    ASSERT_TRUE(set_result.isSuccess());

    const auto get_result = core.getZoom();
    ASSERT_TRUE(get_result.isSuccess());
    EXPECT_EQ(get_result.value(), 2);
}

TEST_F(CoreTests, ZoomOperationsFailWhenNotInitialized) {
    const core::Core core(std::move(camera));

    // Operations should fail when not initialized
    const auto set_result = core.setZoom(50);
    ASSERT_TRUE(set_result.isError());
    EXPECT_THAT(set_result.error(), ::testing::HasSubstr("not initialized"));

    const auto get_result = core.getZoom();
    ASSERT_TRUE(get_result.isError());
    EXPECT_THAT(get_result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, FocusOperations) {
    // Set up all expectations before moving the camera
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(true)); // for shutdown
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, setFocus(1))
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, getFocus())
        .WillOnce(Return(Result<common::types::focus>::success(1u)));
    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::success()));

    // Now create the core with the moved camera
    core::Core core(std::move(camera));
    const auto init_result = core.start();
    ASSERT_TRUE(init_result.isSuccess());

    const auto set_result = core.setFocus(1);
    ASSERT_TRUE(set_result.isSuccess());

    const auto get_result = core.getFocus();
    ASSERT_TRUE(get_result.isSuccess());
    EXPECT_EQ(get_result.value(), 1);
}

TEST_F(CoreTests, FocusOperationsFailWhenNotInitialized) {
    const core::Core core(std::move(camera));

    // Operations should fail when not initialized
    const auto set_result = core.setFocus(50);
    ASSERT_TRUE(set_result.isError());
    EXPECT_THAT(set_result.error(), ::testing::HasSubstr("not initialized"));

    const auto get_result = core.getFocus();
    ASSERT_TRUE(get_result.isError());
    EXPECT_THAT(get_result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, ShutdownSuccess) {
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false))  // initialize
        .WillOnce(Return(true));  // shutdown
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::success()));

    core::Core core(std::move(camera));
    const auto init_result = core.start();
    ASSERT_TRUE(init_result.isSuccess()) << "Failed to initialize: " << init_result.error();

    const auto shutdown_result = core.stop();
    ASSERT_TRUE(shutdown_result.isSuccess()) << "Failed to shut down: " << shutdown_result.error();
}

TEST_F(CoreTests, ShutdownWhenNotInitializedSuccess) {
    core::Core core(std::move(camera));
    const auto shutdown_result = core.stop();
    ASSERT_TRUE(shutdown_result.isSuccess());
}

TEST_F(CoreTests, ShutdownWhenCameraDisconnectFailsFails) {
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::error("Failed to close")));

    core::Core core(std::move(camera));
    const auto init_result = core.start();
    ASSERT_TRUE(init_result.isSuccess());

    const auto shutdown_result = core.stop();
    ASSERT_TRUE(shutdown_result.isError());
    EXPECT_EQ(shutdown_result.error(), "Failed to close");
}

TEST_F(CoreTests, GetInfoSuccess) {
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, getInfo())
        .WillOnce(Return(Result<common::types::info>::success(std::string("Test Camera Info"))));
    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::success()));

    core::Core core(std::move(camera));
    ASSERT_TRUE(core.start().isSuccess());

    const auto info_result = core.getInfo();
    ASSERT_TRUE(info_result.isSuccess());
    EXPECT_EQ(info_result.value(), "Test Camera Info");
}

TEST_F(CoreTests, GetInfoFailsWhenNotInitialized) {
    const core::Core core(std::move(camera));

    const auto info_result = core.getInfo();
    ASSERT_TRUE(info_result.isError());
    EXPECT_THAT(info_result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, EnableAutoFocusSuccess) {
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, enableAutoFocus(true))
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::success()));

    core::Core core(std::move(camera));
    ASSERT_TRUE(core.start().isSuccess());

    const auto af_result = core.enableAutoFocus(true);
    ASSERT_TRUE(af_result.isSuccess());
}

TEST_F(CoreTests, EnableAutoFocusFailsWhenNotInitialized) {
    const core::Core core(std::move(camera));

    const auto af_result = core.enableAutoFocus(true);
    ASSERT_TRUE(af_result.isError());
    EXPECT_THAT(af_result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, StabilizeSuccess) {
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, stabilize(true))
        .WillOnce(Return(Result<void>::success()));
    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::success()));

    core::Core core(std::move(camera));
    ASSERT_TRUE(core.start().isSuccess());

    const auto result = core.stabilize(true);
    ASSERT_TRUE(result.isSuccess());
}

TEST_F(CoreTests, StabilizeFailsWhenNotInitialized) {
    const core::Core core(std::move(camera));

    const auto result = core.stabilize(true);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, GetCapabilitiesSuccess) {
    EXPECT_CALL(*camera, isConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    EXPECT_CALL(*camera, open())
        .WillOnce(Return(Result<void>::success()));

    const common::capabilities::CapabilityList expected {
        common::capabilities::Capability::Zoom,
        common::capabilities::Capability::Focus};

    EXPECT_CALL(*camera, getCapabilities())
        .WillOnce(Return(Result<common::capabilities::CapabilityList>::success(expected)));

    EXPECT_CALL(*camera, close())
        .WillOnce(Return(Result<void>::success()));

    core::Core core(std::move(camera));
    ASSERT_TRUE(core.start().isSuccess());

    const auto result = core.getCapabilities();
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), expected);
}

TEST_F(CoreTests, GetCapabilitiesFailsWhenNotInitialized) {
    const core::Core core(std::move(camera));

    const auto result = core.getCapabilities();
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}