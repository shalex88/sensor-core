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
};

TEST_F(CoreFactoryTests, CreateCoreSuccess) {
    const auto config = createValidConfig();
    const auto core_instance = service::core::CoreFactory::createCore(config);
    ASSERT_NE(nullptr, core_instance);
    ASSERT_TRUE(dynamic_cast<service::core::Core*>(core_instance.get()) != nullptr);
}

TEST_F(CoreFactoryTests, CreateCoreWithEmptyClients) {
    service::common::InfrastructureConfig config;
    const auto core_instance = service::core::CoreFactory::createCore(config);
    ASSERT_NE(nullptr, core_instance);
    ASSERT_TRUE(dynamic_cast<service::core::Core*>(core_instance.get()) != nullptr);
}
