#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "common/config/ConfigManager.h"

#include <filesystem>
#include <fstream>

using namespace testing;
using namespace service;

class ConfigManagerTests : public Test {
protected:
    const std::string test_config_path_ = "test_config.yaml";
    const std::string invalid_config_path_ = "invalid_config.yaml";

    void SetUp() override {
        // Create a valid test config file with instance-based infrastructure
        std::ofstream config_file(test_config_path_);
        config_file << "app:\n";
        config_file << "  name: test\n";
        config_file << "  log_level: info\n";
        config_file << "  api:\n";
        config_file << "    api_type: grpc\n";
        config_file << "    server_address: localhost:50051\n";
        config_file << "  infrastructure:\n";
        config_file << "    clients:\n";
        config_file << "      camera_service:\n";
        config_file << "        instances:\n";
        config_file << "          - id: 0\n";
        config_file << "            address: localhost:50052\n";
        config_file.close();
    }

    void TearDown() override {
        if (std::filesystem::exists(test_config_path_)) {
            std::filesystem::remove(test_config_path_);
        }
        if (std::filesystem::exists(invalid_config_path_)) {
            std::filesystem::remove(invalid_config_path_);
        }
    }

    void createInvalidConfig(const std::string& content) const {
        std::ofstream config_file(invalid_config_path_);
        config_file << content;
        config_file.close();
    }
};

TEST_F(ConfigManagerTests, LoadValidConfig) {
    const common::ConfigManager config(test_config_path_);

    const auto& api_config = config.getApiConfig();
    EXPECT_EQ(api_config.api, "grpc");
    EXPECT_EQ(api_config.server_address, "localhost:50051");

    const auto& infrastructure_config = config.getInfrastructureConfig();
    ASSERT_EQ(infrastructure_config.clients.size(), 1);
    ASSERT_TRUE(infrastructure_config.clients.contains("camera_service"));

    const auto& camera_client = infrastructure_config.clients.at("camera_service");
    ASSERT_EQ(camera_client.instances.size(), 1);
    EXPECT_EQ(camera_client.instances.front().id, 0u);
    EXPECT_EQ(camera_client.instances.front().address, "localhost:50052");

    const auto& log_level = config.getLogLevel();
    const auto& app_name = config.getAppName();
    EXPECT_EQ(app_name, "test");
    EXPECT_EQ(log_level, "info");
}

TEST_F(ConfigManagerTests, ThrowsOnNonexistentFile) {
    EXPECT_THROW({ service::common::ConfigManager config("nonexistent.yaml");
                 }, std::runtime_error);
}

TEST_F(ConfigManagerTests, ThrowsOnInvalidYaml) {
    createInvalidConfig("invalid: : yaml : content");
    EXPECT_THROW({ service::common::ConfigManager config(invalid_config_path_);
                 }, std::runtime_error);
}

TEST_F(ConfigManagerTests, ThrowsOnInvalidApiType) {
    createInvalidConfig("app:\n  name: test\n  log_level: info\n  api:\n    api_type: invalid_api\n    server_address: localhost:50051\n  infrastructure:\n    clients:\n      camera_service:\n        instances:\n          - id: 0\n            address: localhost:50052");
    EXPECT_THROW({ service::common::ConfigManager config(invalid_config_path_);
    }, std::runtime_error);
}

TEST_F(ConfigManagerTests, ThrowsOnMissingServerAddress) {
    createInvalidConfig("app:\n  name: test\n  log_level: info\n  api:\n    api_type: grpc\n  infrastructure:\n    clients:\n      camera_service:\n        instances:\n          - id: 0\n            address: localhost:50052");
    EXPECT_THROW({ service::common::ConfigManager config(invalid_config_path_);
    }, std::runtime_error);
}

TEST_F(ConfigManagerTests, ThrowsOnInvalidServerAddressFormat) {
    createInvalidConfig("app:\n  name: test\n  log_level: info\n  api:\n    api_type: grpc\n    server_address: invalid_address\n  infrastructure:\n    clients:\n      camera_service:\n        instances:\n          - id: 0\n            address: localhost:50052");
    EXPECT_THROW({ service::common::ConfigManager config(invalid_config_path_);
    }, std::runtime_error);
}

TEST_F(ConfigManagerTests, ThrowsOnEmptyClientAddress) {
    createInvalidConfig("app:\n  name: test\n  log_level: info\n  api:\n    api_type: grpc\n    server_address: localhost:50051\n  infrastructure:\n    clients:\n      camera_service:\n        instances:\n          - id: 0\n            address: \"\"");
    EXPECT_THROW({
        service::common::ConfigManager config(invalid_config_path_);
    }, std::runtime_error);
}

TEST_F(ConfigManagerTests, HandlesAppName) {
    createInvalidConfig("app:\n  name: demo\n  log_level: info\n  api:\n    api_type: grpc\n    server_address: localhost:50051\n  infrastructure:\n    clients:\n      camera_service:\n        instances:\n          - id: 0\n            address: localhost:50052");
    const service::common::ConfigManager config(invalid_config_path_);

    const auto& name = config.getAppName();
    EXPECT_EQ(name, "demo");
}

TEST_F(ConfigManagerTests, HandlesLogLevel) {
    createInvalidConfig("app:\n  name: test\n  log_level: info\n  api:\n    api_type: grpc\n    server_address: localhost:50051\n  infrastructure:\n    clients:\n      camera_service:\n        instances:\n          - id: 0\n            address: localhost:50052");
    const service::common::ConfigManager config(invalid_config_path_);

    const auto& log_level = config.getLogLevel();
    EXPECT_EQ(log_level, "info");
}

TEST_F(ConfigManagerTests, ThrowsOnMissingAppName) {
    createInvalidConfig("app:\n  log_level: info\n  api:\n    api_type: grpc\n    server_address: localhost:50051\n  infrastructure:\n    clients:\n      camera_service:\n        instances:\n          - id: 0\n            address: localhost:50052");
    EXPECT_THROW({
        service::common::ConfigManager config(invalid_config_path_);
    }, std::runtime_error);
}

TEST_F(ConfigManagerTests, ThrowsOnMissingLogLevel) {
    createInvalidConfig("app:\n  name: test\n  api:\n    api_type: grpc\n    server_address: localhost:50051\n  infrastructure:\n    clients:\n      camera_service:\n        instances:\n          - id: 0\n            address: localhost:50052");
    EXPECT_THROW({
        service::common::ConfigManager config(invalid_config_path_);
    }, std::runtime_error);
}

TEST_F(ConfigManagerTests, ThrowsOnInvalidLogLevel) {
    createInvalidConfig("app:\n  name: test\n  log_level: invalid_level\n  api:\n    api_type: grpc\n    server_address: localhost:50051\n  infrastructure:\n    clients:\n      camera_service:\n        instances:\n          - id: 0\n            address: localhost:50052");
    EXPECT_THROW({
        service::common::ConfigManager config(invalid_config_path_);
    }, std::runtime_error);
}

TEST_F(ConfigManagerTests, HandlesEmptyClients) {
    createInvalidConfig("app:\n  name: test\n  log_level: info\n  api:\n    api_type: grpc\n    server_address: localhost:50051\n  infrastructure:\n    clients: {}");
    const service::common::ConfigManager config(invalid_config_path_);

    const auto& infrastructure_config = config.getInfrastructureConfig();
    EXPECT_EQ(infrastructure_config.clients.size(), 0);
}

TEST_F(ConfigManagerTests, HandlesNoInfrastructureSection) {
    createInvalidConfig("app:\n  name: test\n  log_level: info\n  api:\n    api_type: grpc\n    server_address: localhost:50051");
    const service::common::ConfigManager config(invalid_config_path_);

    const auto& infrastructure_config = config.getInfrastructureConfig();
    EXPECT_EQ(infrastructure_config.clients.size(), 0);
}
