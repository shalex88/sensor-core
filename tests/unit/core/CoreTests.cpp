#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "core/Core.h"
#include "common/config/ConfigManager.h"
#include "common/types/Result.h"

using namespace testing;

class CoreTests : public Test {
protected:
    service::common::InfrastructureConfig createValidConfig() {
        service::common::InfrastructureConfig config;
        service::common::ClientConfig camera_service;
        service::common::ServiceInstance instance;
        instance.id = 1;
        instance.server = "localhost";
        instance.port = 50052;
        camera_service.instances.push_back(instance);
        config.clients.emplace("camera_service", camera_service);
        return config;
    }

    service::common::InfrastructureConfig createEmptyConfig() {
        return service::common::InfrastructureConfig{};
    }
};

TEST_F(CoreTests, CanBeCreatedWithValidConfig) {
    const auto config = createValidConfig();
    EXPECT_NO_THROW(service::core::Core core(config));
}

TEST_F(CoreTests, CanBeCreatedWithEmptyConfig) {
    const auto config = createEmptyConfig();
    EXPECT_NO_THROW(service::core::Core core(config));
}

TEST_F(CoreTests, StartsSuccessfully) {
    const auto config = createValidConfig();
    service::core::Core core(config);
    const auto result = core.start();
    ASSERT_TRUE(result.isSuccess()) << "Failed to start: " << result.error();
}

TEST_F(CoreTests, StopsSuccessfully) {
    const auto config = createValidConfig();
    service::core::Core core(config);
    ASSERT_TRUE(core.start().isSuccess());
    const auto result = core.stop();
    ASSERT_TRUE(result.isSuccess()) << "Failed to stop: " << result.error();
}

TEST_F(CoreTests, StopsWhenNotStartedSuccessfully) {
    const auto config = createValidConfig();
    service::core::Core core(config);
    const auto result = core.stop();
    ASSERT_TRUE(result.isSuccess());
}

TEST_F(CoreTests, ZoomOperationsFailWhenNotInitialized) {
    const auto config = createValidConfig();
    const service::core::Core core(config);

    // Operations should fail when not initialized (not started)
    const auto set_result = core.setZoom(1, 50);
    ASSERT_TRUE(set_result.isError());
    EXPECT_THAT(set_result.error(), ::testing::HasSubstr("not initialized"));

    const auto get_result = core.getZoom(1);
    ASSERT_TRUE(get_result.isError());
    EXPECT_THAT(get_result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, FocusOperationsFailWhenNotInitialized) {
    const auto config = createValidConfig();
    const service::core::Core core(config);

    const auto set_result = core.setFocus(1, 50);
    ASSERT_TRUE(set_result.isError());
    EXPECT_THAT(set_result.error(), ::testing::HasSubstr("not initialized"));

    const auto get_result = core.getFocus(1);
    ASSERT_TRUE(get_result.isError());
    EXPECT_THAT(get_result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, InfoOperationFailsWhenNotInitialized) {
    const auto config = createValidConfig();
    const service::core::Core core(config);

    const auto result = core.getInfo(1);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, AutoFocusOperationFailsWhenNotInitialized) {
    const auto config = createValidConfig();
    const service::core::Core core(config);

    const auto result = core.enableAutoFocus(1, true);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, StabilizeOperationFailsWhenNotInitialized) {
    const auto config = createValidConfig();
    const service::core::Core core(config);

    const auto result = core.stabilize(1, true);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}

TEST_F(CoreTests, GetCapabilitiesFailsWhenNotInitialized) {
    const auto config = createValidConfig();
    const service::core::Core core(config);

    const auto result = core.getCapabilities(1);
    ASSERT_TRUE(result.isError());
    EXPECT_THAT(result.error(), ::testing::HasSubstr("not initialized"));
}