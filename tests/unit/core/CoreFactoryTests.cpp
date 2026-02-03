#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "core/CoreFactory.h"
#include "common/config/ConfigManager.h"

#include "core/Core.h"
#include "common/types/Result.h"

using namespace testing;

class CoreFactoryTests : public Test {
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
};

TEST_F(CoreFactoryTests, CreateCoreSuccess) {
    const auto config = createValidConfig();
    const auto core = core::CoreFactory::createCore(config);
    ASSERT_NE(nullptr, core);
    ASSERT_TRUE(dynamic_cast<core::Core*>(core.get()) != nullptr);
}

TEST_F(CoreFactoryTests, CreateCoreWithEmptyClients) {
    common::InfrastructureConfig config;
    const auto core = core::CoreFactory::createCore(config);
    ASSERT_NE(nullptr, core);
    ASSERT_TRUE(dynamic_cast<core::Core*>(core.get()) != nullptr);
}

