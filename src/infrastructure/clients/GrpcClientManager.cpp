#include "infrastructure/clients/GrpcClientManager.h"

#include "infrastructure/clients/CameraServiceClient.h"
#include "common/logger/Logger.h"

namespace service::infrastructure {
    GrpcClientManager::GrpcClientManager(const common::InfrastructureConfig& config)
        : config_(config) {
    }

    GrpcClientManager::~GrpcClientManager() {
        shutdown();
    }

    void GrpcClientManager::initialize() {
        LOG_DEBUG("Initializing gRPC clients from configuration");

        // Check if camera_service is configured
        if (config_.clients.count("camera_service") == 0) {
            LOG_WARN("camera_service not found in configuration");
            return;
        }

        const auto& camera_config = config_.clients.at("camera_service");

        if (camera_config.instances.empty()) {
            LOG_ERROR("camera_service has no instances configured");
            throw std::runtime_error("camera_service must have at least one instance");
        }

        LOG_DEBUG("Initializing {} camera_service instance(s)", camera_config.instances.size());

        try {
            for (const auto& instance : camera_config.instances) {
                LOG_DEBUG("Creating camera_service client for instance {} at {}", instance.id, instance.address);

                auto channel = createChannel(instance.address);
                // Cast Channel to ChannelInterface for the client
                std::shared_ptr<grpc::ChannelInterface> channel_interface =
                    std::static_pointer_cast<grpc::ChannelInterface>(channel);

                camera_channels_[instance.id] = channel;
                camera_clients_[instance.id] = std::make_unique<CameraServiceClient>(channel_interface);

                LOG_DEBUG("camera_service instance {} initialized successfully", instance.id);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize camera_service clients: {}", e.what());
            // Clean up any partially initialized instances
            shutdown();
            throw;
        }
    }

    void GrpcClientManager::shutdown() {
        LOG_DEBUG("Shutting down gRPC clients");

        // Clear all camera client instances
        for (const auto& [camera_id, _] : camera_clients_) {
            LOG_DEBUG("Closing camera_service client instance {}", camera_id);
        }
        camera_clients_.clear();

        // Clear all camera channel instances
        for (const auto& [camera_id, channel] : camera_channels_) {
            if (channel) {
                // Shutdown the channel - gRPC will attempt graceful shutdown
                channel->GetState(true);
                LOG_DEBUG("camera_service channel {} closed", camera_id);
            }
        }
        camera_channels_.clear();
    }

    ICameraServiceClient* GrpcClientManager::getCameraServiceClient(uint32_t camera_id) const {
        // Validate camera_id range [0-3]
        if (camera_id > 3) {
            throw std::invalid_argument("Invalid camera_id: " + std::to_string(camera_id) +
                                      ". Valid range is [0-3]");
        }

        auto it = camera_clients_.find(camera_id);
        if (it == camera_clients_.end()) {
            LOG_WARN("Camera service client {} not initialized", camera_id);
            return nullptr;
        }

        return it->second.get();
    }

    std::shared_ptr<grpc::Channel> GrpcClientManager::createChannel(const std::string& address) {
        if (address.empty()) {
            throw std::invalid_argument("Service address cannot be empty");
        }

        LOG_DEBUG("Creating gRPC channel to: {}", address);

        // Create channel with default arguments
        // Consider adding channel arguments for:
        // - Connection timeout
        // - Keep-alive settings
        // - Max message size
        auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());

        if (!channel) {
            throw std::runtime_error("Failed to create gRPC channel to " + address);
        }

        LOG_DEBUG("gRPC channel created successfully");
        return channel;
    }
} // namespace service::infrastructure
