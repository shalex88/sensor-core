#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "core/CoreFactory.h"
#include "common/config/ConfigManager.h"

#include "core/Core.h"
#include "../../Mocks.h"
#include "common/types/Result.h"

class CoreFactoryTests : public Test {
protected:
    CoreFactoryTests() {
        core_ = std::make_unique<MockCameraHal>();
    }
    std::unique_ptr<MockCameraHal> core_;
};

TEST_F(CoreFactoryTests, CreateCameraCoreSuccess) {
    common::CoreConfig config;
    config.camera = "core";

    const auto core = core::CoreFactory::createCore(std::move(core_), config);
    ASSERT_NE(nullptr, core);
    ASSERT_TRUE(dynamic_cast<core::Core*>(core.get()) != nullptr);
}

TEST_F(CoreFactoryTests, ThrowsOnUnknownType) {
    common::CoreConfig config;
    config.camera = "invalid_camera";

    EXPECT_THROW(
        core::CoreFactory::createCore(std::move(core_), config),
        std::invalid_argument
    );
}

TEST_F(CoreFactoryTests, ThrowsOnNullCamera) {
    common::CoreConfig config;
    config.camera = "nfov";

    EXPECT_THROW(
        core::CoreFactory::createCore(nullptr, config),
        std::invalid_argument
    );
}
