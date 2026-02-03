#include "Core.h"

#include "common/logger/Logger.h"
#include "infrastructure/clients/GrpcClientManager.h"
#include "infrastructure/clients/ICameraServiceClient.h"

namespace service::core {
    Core::Core(const common::InfrastructureConfig& infrastructure_config)
        : infrastructure_config_(infrastructure_config), is_running_(false) {
    }

    Core::~Core() {
        if (stop().isError()) {
            LOG_ERROR("Failed to shut down Core properly");
        }
    }

    Result<void> Core::start() {
        LOG_DEBUG("Starting Core...");

        if (is_running_) {
            return Result<void>::error("Core is already running");
        }

        try {
            // Initialize gRPC client manager
            client_manager_ = std::make_unique<infrastructure::GrpcClientManager>(infrastructure_config_);
            client_manager_->initialize();

            is_running_ = true;
            LOG_DEBUG("Core started successfully");
            return Result<void>::success();
        } catch (const std::exception& e) {
            return Result<void>::error(std::string("Failed to start Core: ") + e.what());
        }
    }

    Result<void> Core::stop() {
        if (!isRunning()) {
            return Result<void>::success();
        }

        LOG_DEBUG("Stopping Core...");

        try {
            if (client_manager_) {
                client_manager_->shutdown();
                client_manager_.reset();
            }
            is_running_ = false;
            LOG_DEBUG("Core stopped successfully");
            return Result<void>::success();
        } catch (const std::exception& e) {
            return Result<void>::error(std::string("Failed to stop Core: ") + e.what());
        }
    }

    bool Core::isRunning() const {
        return is_running_;
    }

    Result<void> Core::setZoom(uint32_t camera_id, const common::types::zoom zoom_level) const {
        if (!isRunning()) {
            return Result<void>::error("Core is not initialized");
        }

        try {
            auto* client = client_manager_->getCameraServiceClient(camera_id);
            if (!client) {
                return Result<void>::error("camera_service client for instance " + std::to_string(camera_id) +
                                          " is not available");
            }

            return client->setZoom(zoom_level);
        } catch (const std::exception& e) {
            return Result<void>::error(std::string("setZoom failed: ") + e.what());
        }
    }

    Result<common::types::zoom> Core::getZoom(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<common::types::zoom>::error("Core is not initialized");
        }

        try {
            auto* client = client_manager_->getCameraServiceClient(camera_id);
            if (!client) {
                return Result<common::types::zoom>::error("camera_service client for instance " +
                                                        std::to_string(camera_id) + " is not available");
            }

            return client->getZoom();
        } catch (const std::exception& e) {
            return Result<common::types::zoom>::error(std::string("getZoom failed: ") + e.what());
        }
    }

    Result<void> Core::goToMinZoom(uint32_t camera_id) const {
        return setZoom(camera_id, common::types::MIN_NORMALIZED_ZOOM);
    }

    Result<void> Core::goToMaxZoom(uint32_t camera_id) const {
        return setZoom(camera_id, common::types::MAX_NORMALIZED_ZOOM);
    }

    Result<void> Core::setFocus(uint32_t camera_id, const common::types::focus focus_value) const {
        if (!isRunning()) {
            return Result<void>::error("Core is not initialized");
        }

        try {
            auto* client = client_manager_->getCameraServiceClient(camera_id);
            if (!client) {
                return Result<void>::error("camera_service client for instance " + std::to_string(camera_id) +
                                          " is not available");
            }

            return client->setFocus(focus_value);
        } catch (const std::exception& e) {
            return Result<void>::error(std::string("setFocus failed: ") + e.what());
        }
    }

    Result<common::types::focus> Core::getFocus(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<common::types::focus>::error("Core is not initialized");
        }

        try {
            auto* client = client_manager_->getCameraServiceClient(camera_id);
            if (!client) {
                return Result<common::types::focus>::error("camera_service client for instance " +
                                                         std::to_string(camera_id) + " is not available");
            }

            return client->getFocus();
        } catch (const std::exception& e) {
            return Result<common::types::focus>::error(std::string("getFocus failed: ") + e.what());
        }
    }

    Result<void> Core::enableAutoFocus(uint32_t camera_id, const bool on) const {
        if (!isRunning()) {
            return Result<void>::error("Core is not initialized");
        }

        try {
            auto* client = client_manager_->getCameraServiceClient(camera_id);
            if (!client) {
                return Result<void>::error("camera_service client for instance " + std::to_string(camera_id) +
                                          " is not available");
            }

            return client->enableAutoFocus(on);
        } catch (const std::exception& e) {
            return Result<void>::error(std::string("enableAutoFocus failed: ") + e.what());
        }
    }

    Result<common::types::info> Core::getInfo(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<common::types::info>::error("Core is not initialized");
        }

        try {
            auto* client = client_manager_->getCameraServiceClient(camera_id);
            if (!client) {
                return Result<common::types::info>::error("camera_service client for instance " +
                                                        std::to_string(camera_id) + " is not available");
            }

            return client->getInfo();
        } catch (const std::exception& e) {
            return Result<common::types::info>::error(std::string("getInfo failed: ") + e.what());
        }
    }

    Result<void> Core::stabilize(uint32_t camera_id, const bool on) const {
        if (!isRunning()) {
            return Result<void>::error("Core is not initialized");
        }

        try {
            auto* client = client_manager_->getCameraServiceClient(camera_id);
            if (!client) {
                return Result<void>::error("camera_service client for instance " + std::to_string(camera_id) +
                                          " is not available");
            }

            return client->stabilize(on);
        } catch (const std::exception& e) {
            return Result<void>::error(std::string("stabilize failed: ") + e.what());
        }
    }

    Result<common::capabilities::CapabilityList> Core::getCapabilities(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<common::capabilities::CapabilityList>::error("Core is not initialized");
        }

        try {
            auto* client = client_manager_->getCameraServiceClient(camera_id);
            if (!client) {
                return Result<common::capabilities::CapabilityList>::error("camera_service client for instance " +
                                                                          std::to_string(camera_id) +
                                                                          " is not available");
            }

            return client->getCapabilities();
        } catch (const std::exception& e) {
            return Result<common::capabilities::CapabilityList>::error(std::string("getCapabilities failed: ") +
                                                                       e.what());
        }
    }
} // namespace service::core
