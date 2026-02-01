#include "ConfigManager.h"

#include <set>
#include <yaml-cpp/yaml.h>

namespace service::common {
    namespace {
        EndpointConfig parseEndpointNode(const YAML::Node& node) {
            EndpointConfig endpoint;

            if (!node) {
                return endpoint;
            }

            if (node.IsScalar()) {
                endpoint.address = node.as<std::string>();
                return endpoint;
            }

            if (node["address"]) {
                endpoint.address = node["address"].as<std::string>();
            }

            if (node["configuration"]) {
                const auto& configuration_node = node["configuration"];
                if (!configuration_node.IsMap()) {
                    throw std::runtime_error("Endpoint configuration must be a key/value map");
                }

                for (const auto& entry : configuration_node) {
                    const auto key = entry.first.as<std::string>();
                    const auto value = entry.second.as<std::string>();
                    endpoint.configuration.emplace(key, value);
                }
            }

            return endpoint;
        }
    } // unnamed namespace

    void ApiConfig::validate() const {
        static const std::set<std::string> valid_apis{"grpc"};

        if (api.empty()) {
            throw std::runtime_error("API type cannot be empty");
        }
        if (!valid_apis.contains(api)) {
            throw std::runtime_error("Invalid API type: " + api);
        }
        if (server_address.empty()) {
            throw std::runtime_error("Server address cannot be empty");
        }
        if (server_address.find(':') == std::string::npos) {
            throw std::runtime_error("Server address must include port (format: host:port)");
        }
    }

    void CoreConfig::validate() const {
        static const std::set<std::string> valid_cameras{"core"};

        if (camera.empty()) {
            throw std::runtime_error("Camera type cannot be empty");
        }
    }

    void EndpointConfig::validate() const {
        if (address.empty()) {
            throw std::runtime_error("Endpoint address cannot be empty");
        }
    }

    void InfrastructureConfig::validate() const {
        static const std::set<std::string> valid_cameras{"sony", "adimec", "mwir", "fake_advanced", "fake_simple"};

        if (camera.empty()) {
            throw std::runtime_error("Infrastructure camera type cannot be empty");
        }
        if (!valid_cameras.contains(camera)) {
            throw std::runtime_error("Invalid infrastructure camera type: " + camera);
        }

        for (const auto& endpoint : endpoints) {
            endpoint.validate();
        }

        if (camera == "adimec" && endpoints.size() != 2) {
            throw std::runtime_error("Adimec camera requires exactly 2 endpoints");
        }
    }

    void AppConfig::validate() const {
        static const std::set<std::string> valid_log_levels{"trace", "debug", "info", "warn", "error", "critical"};

        api_config.validate();
        core_config.validate();
        infrastructure_config.validate();

        if (log_level.empty()) {
            throw std::runtime_error("Log level cannot be empty");
        }
        if (!valid_log_levels.contains(log_level)) {
            throw std::runtime_error("Invalid log level: " + log_level);
        }
        if (name.empty()) {
            throw std::runtime_error("App name cannot be empty");
        }

        if (api_config.server_address == core_config.camera) {
            throw std::runtime_error("API server address cannot be the same as camera type");
        }
    }

    ConfigManager::ConfigManager(const std::string& filename) : app_config_(std::make_unique<AppConfig>()) {
        if (!std::filesystem::exists(filename)) {
            throw std::runtime_error("Configuration file does not exist: " + filename);
        }
        loadFromFile(filename);
        validateConfiguration();
    }

    void ConfigManager::loadFromFile(const std::filesystem::path& filename) const {
        try {
            if (const YAML::Node config = YAML::LoadFile(filename); config["app"]) {
                const auto& app_node = config["app"];
                loadApiConfig(app_node);
                loadCoreConfig(app_node);
                loadInfrastructureConfig(app_node);
                loadAppConfig(app_node);
            }
        }
        catch (const YAML::Exception& e) {
            throw std::runtime_error("YAML parsing error: " + std::string(e.what()));
        }
    }

    void ConfigManager::loadApiConfig(const YAML::Node& app_node) const {
        if (app_node["api"]) {
            const auto& api_node = app_node["api"];
            if (api_node["api_type"]) {
                app_config_->api_config.api = api_node["api_type"].as<std::string>();
            }
            if (api_node["server_address"]) {
                app_config_->api_config.server_address = api_node["server_address"].as<std::string>();
            }
        }
    }

    void ConfigManager::loadCoreConfig(const YAML::Node& app_node) const {
        if (app_node["core"]) {
            if (const auto& core_node = app_node["core"]; core_node["camera"]) {
                app_config_->core_config.camera = core_node["camera"].as<std::string>();
            }
        }
    }

    void ConfigManager::loadInfrastructureConfig(const YAML::Node& app_node) const {
        if (!app_node["infrastructure"]) {
            return;
        }

        const auto& infrastructure_node = app_node["infrastructure"];

        if (infrastructure_node["camera"]) {
            app_config_->infrastructure_config.camera = infrastructure_node["camera"].as<std::string>();
        }

        if (infrastructure_node["video_channel"]) {
            app_config_->infrastructure_config.video_channel = infrastructure_node["video_channel"].as<int>();
        }

        app_config_->infrastructure_config.endpoints.clear();

        if (!infrastructure_node["endpoints"]) {
            return;
        }

        const auto& endpoints_node = infrastructure_node["endpoints"];
        if (!endpoints_node.IsSequence()) {
            throw std::runtime_error("Endpoints must be a list");
        }

        for (const auto& endpoint_node : endpoints_node) {
            app_config_->infrastructure_config.endpoints.push_back(parseEndpointNode(endpoint_node));
        }
    }

    void ConfigManager::loadAppConfig(const YAML::Node& app_node) const {
        if (app_node["log_level"]) {
            app_config_->log_level = app_node["log_level"].as<std::string>();
        }
        if (app_node["name"]) {
            app_config_->name = app_node["name"].as<std::string>();
        }
    }

    const ApiConfig& ConfigManager::getApiConfig() const {
        return app_config_->api_config;
    }

    const CoreConfig& ConfigManager::getCoreConfig() const {
        return app_config_->core_config;
    }

    const InfrastructureConfig& ConfigManager::getInfrastructureConfig() const {
        return app_config_->infrastructure_config;
    }

    const std::string& ConfigManager::getLogLevel() const {
        return app_config_->log_level;
    }

    const std::string& ConfigManager::getAppName() const {
        return app_config_->name;
    }

    void ConfigManager::validateConfiguration() const {
        if (!app_config_) {
            throw std::runtime_error("Configuration not initialized");
        }
        app_config_->validate();
    }
} // namespace service::common
