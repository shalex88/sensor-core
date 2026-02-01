#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "api/ApiControllerFactory.h"
#include "common/config/ConfigManager.h"
#include "api/ApiController.h"
#include "core/ICore.h"
#include "common/types/Result.h"
#include "../../Mocks.h"

class ApiControllerFactoryTests : public Test {
protected:
    ApiControllerFactoryTests() {
        core_ = std::make_unique<CoreMock>();
    }

    std::string server_address = "50051";
    std::unique_ptr<core::ICore> core_;
};

TEST_F(ApiControllerFactoryTests, CreateGrpcServiceSuccess) {
    common::ApiConfig config;
    config.api = "grpc";
    config.server_address = "localhost:50051";

    const auto service = api::ApiControllerFactory::createController(std::move(core_), config);
    ASSERT_NE(nullptr, service);
    ASSERT_TRUE(service.get() != nullptr);
}

TEST_F(ApiControllerFactoryTests, ThrowsOnUnknownType) {
    common::ApiConfig config;
    config.api = "invalid_api";
    config.server_address = "localhost:50051";

    EXPECT_THROW(
        api::ApiControllerFactory::createController(std::move(core_), config),
        std::invalid_argument
    );
}
