#include "ConfigManager.h"

#include <set>
#include <yaml-cpp/yaml.h>

namespace service::common {

    void ApiConfig::validate() const {
        static const std::set<std::string> valid_apis{"grpc", "rest"};

        if (api.empty()) {
            throw std::runtime_error("API type cannot be empty");
        }
        if (!valid_apis.contains(api)) {
            throw std::runtime_error("Invalid API type: " + api);
        }
        if (server.empty()) {
            throw std::runtime_error("Server cannot be empty");
        }
        if (port == 0) {
            throw std::runtime_error("Port cannot be zero");
        }
    }

    void CoreConfig::validate() const {
        // No validation needed for CoreConfig after removing camera dependency
    }

    void ServiceInstance::validate() const {
        if (server.empty()) {
            throw std::runtime_error("Service instance server cannot be empty");
        }
        if (port == 0) {
            throw std::runtime_error("Service instance port cannot be zero");
        }
    }

    void ClientConfig::validate() const {
        if (instances.empty()) {
            throw std::runtime_error("Client must have at least one instance configured");
        }
        for (const auto& instance : instances) {
            instance.validate();
        }
    }

    void InfrastructureConfig::validate() const {
        // Validate all clients
        for (const auto& [name, client] : clients) {
            client.validate();
        }

        const auto camera_it = clients.find("camera_service");
        const auto video_it = clients.find("video_service");
        if (camera_it != clients.end() && video_it != clients.end()) {
            std::set<uint32_t> camera_ids;
            std::set<uint32_t> video_ids;

            for (const auto& instance : camera_it->second.instances) {
                camera_ids.insert(instance.id);
            }
            for (const auto& instance : video_it->second.instances) {
                video_ids.insert(instance.id);
            }

            if (camera_ids != video_ids) {
                throw std::runtime_error(
                    "camera_service and video_service must have matching instance IDs");
            }
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
            if (api_node["server"]) {
                app_config_->api_config.server = api_node["server"].as<std::string>();
            }
            if (api_node["port"]) {
                app_config_->api_config.port = api_node["port"].as<uint16_t>();
            }
        }
    }

    void ConfigManager::loadInfrastructureConfig(const YAML::Node& app_node) const {
        if (!app_node["infrastructure"]) {
            return;
        }

        const auto& infrastructure_node = app_node["infrastructure"];

        if (!infrastructure_node["clients"]) {
            return;
        }

        const auto& clients_node = infrastructure_node["clients"];
        if (!clients_node.IsMap()) {
            throw std::runtime_error("Clients must be a key/value map");
        }

        app_config_->infrastructure_config.clients.clear();

        for (const auto& client_entry : clients_node) {
            const auto client_name = client_entry.first.as<std::string>();
            const auto& client_node = client_entry.second;

            ClientConfig client_config;

            // Parse instances array (new structure)
            if (client_node["instances"] && client_node["instances"].IsSequence()) {
                const auto& instances_node = client_node["instances"];
                for (const auto& instance_node : instances_node) {
                    ServiceInstance instance;
                    if (instance_node["id"]) {
                        instance.id = instance_node["id"].as<uint32_t>();
                    } else {
                        throw std::runtime_error("Service instance must have an 'id' field");
                    }
                    if (instance_node["server"]) {
                        instance.server = instance_node["server"].as<std::string>();
                    } else {
                        throw std::runtime_error("Service instance must have a 'server' field");
                    }
                    if (instance_node["port"]) {
                        instance.port = instance_node["port"].as<uint16_t>();
                    } else {
                        throw std::runtime_error("Service instance must have a 'port' field");
                    }
                    client_config.instances.emplace_back(instance);
                }
            } else if (client_node["server"] && client_node["port"]) {
                // Legacy: single server and port (for backwards compatibility)
                ServiceInstance instance;
                instance.id = 0;
                instance.server = client_node["server"].as<std::string>();
                instance.port = client_node["port"].as<uint16_t>();
                client_config.instances.emplace_back(instance);
            }

            app_config_->infrastructure_config.clients.emplace(client_name, client_config);
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
