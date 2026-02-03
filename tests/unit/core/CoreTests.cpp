#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "core/Core.h"
#include "common/config/ConfigManager.h"
#include "common/types/Result.h"

using namespace testing;

class CoreTests : public Test {
protected:
    common::InfrastructureConfig createValidConfig() {
        common::InfrastructureConfig config;
        common::ClientConfig camera_service;
        common::ServiceInstance instance;
        instance.id = 0;
        instance.address = "localhost:50052";
        camera_service.instances.push_back(instance);
        config.clients.emplace("camera_service", camera_service);
        return config;
    }

    common::InfrastructureConfig createEmptyConfig() {
        return common::InfrastructureConfig{};
    }
};

TEST_F(CoreTests, CanBeCreatedWithValidConfig) {
    const auto config = createValidConfig();
    EXPECT_NO_THROW(core::Core core(config));
}

TEST_F(CoreTests, CanBeCreatedWithEmptyConfig) {
    const auto config = createEmptyConfig();
    EXPECT_NO_THROW(core::Core core(config));
}

TEST_F(CoreTests, StartsSuccessfully) {
    const auto config = createValidConfig();
    core::Core core(config);
    const auto result = core.start();
    ASSERT_TRUE(result.isSuccess()) << "Failed to start: " << result.error();
}

TEST_F(CoreTests, StopsSuccessfully) {
    const auto config = createValidConfig();
    core::Core core(config);
    ASSERT_TRUE(core.start().isSuccess());
    const auto result = core.stop();
    ASSERT_TRUE(result.isSuccess()) << "Failed to stop: " << result.error();
}

TEST_F(CoreTests, StopsWhenNotStartedSuccessfully) {
    const auto config = createValidConfig();
    core::Core core(config);
    const auto result = core.stop();
    ASSERT_TRUE(result.isSuccess());
}

TEST_F(CoreTests, ZoomOperationsFailWhenNotInitialized) {
    const auto config = createValidConfig();
    const core::Core core(config);

    // Operations should fail when not initialized (not started)
    const auto set_result = core.setZoom(0, 50);
    ASSERT_TRUE(set_result.isError());
    EXPECT_THAT(set_result.error(), ::testing::HasSubstr("not initialized"));

    const auto get_result = core.getZoom(0);
    ASSERT_TRUE(get_result.isError());
    EXPECT_THAT(get_result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, FocusOperationsFailWhenNotInitialized) {
    const auto config = createValidConfig();
    const core::Core core(config);

    const auto set_result = core.setFocus(0, 50);
    ASSERT_TRUE(set_result.isError());
    EXPECT_THAT(set_result.error(), ::testing::HasSubstr("not initialized"));

    const auto get_result = core.getFocus(0);
    ASSERT_TRUE(get_result.isError());
    EXPECT_THAT(get_result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, InfoOperationFailsWhenNotInitialized) {
    const auto config = createValidConfig();
    const core::Core core(config);

    const auto result = core.getInfo(0);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, AutoFocusOperationFailsWhenNotInitialized) {
    const auto config = createValidConfig();
    const core::Core core(config);

    const auto result = core.enableAutoFocus(0, true);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, StabilizeOperationFailsWhenNotInitialized) {
    const auto config = createValidConfig();
    const core::Core core(config);

    const auto result = core.stabilize(0, true);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, GetCapabilitiesFailsWhenNotInitialized) {
    const auto config = createValidConfig();
    const core::Core core(config);

    const auto result = core.getCapabilities(0);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}