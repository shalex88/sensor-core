#include "infrastructure/clients/GrpcClientManager.h"

#include "infrastructure/clients/CameraServiceClient.h"
#include "infrastructure/clients/VideoServiceClient.h"
#include "infrastructure/clients/InstanceRouter.h"
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

        try {
            // Initialize camera_service if configured
            initializeService<ICameraServiceClient>(
                "camera_service",
                camera_channels_,
                camera_clients_,
                [](std::shared_ptr<grpc::ChannelInterface> ch) {
                    return std::make_unique<CameraServiceClient>(ch);
                }
            );

            // Initialize video_service if configured
            initializeService<IVideoServiceClient>(
                "video_service",
                video_channels_,
                video_clients_,
                [](std::shared_ptr<grpc::ChannelInterface> ch) {
                    return std::make_unique<VideoServiceClient>(ch);
                }
            );
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize gRPC clients: {}", e.what());
            // Clean up any partially initialized instances
            shutdown();
            throw;
        }
    }

    void GrpcClientManager::shutdown() {
        LOG_DEBUG("Shutting down gRPC clients");

        shutdownService<ICameraServiceClient>("camera_service", camera_channels_, camera_clients_);
        shutdownService<IVideoServiceClient>("video_service", video_channels_, video_clients_);
    }

    ICameraServiceClient* GrpcClientManager::getCameraServiceClient(uint32_t instance_id) const {
        if (!InstanceRouter<ICameraServiceClient>::isConfigured(camera_clients_)) {
            throw std::invalid_argument("camera_service not configured");
        }

        return InstanceRouter<ICameraServiceClient>::getClient("camera_service", instance_id, camera_clients_);
    }

    IVideoServiceClient* GrpcClientManager::getVideoServiceClient(uint32_t instance_id) const {
        if (!InstanceRouter<IVideoServiceClient>::isConfigured(video_clients_)) {
            throw std::invalid_argument("video_service not configured");
        }

        return InstanceRouter<IVideoServiceClient>::getClient("video_service", instance_id, video_clients_);
    }

    template<typename ClientType>
    void GrpcClientManager::initializeService(
        const std::string& service_name,
        std::unordered_map<uint32_t, std::shared_ptr<grpc::Channel>>& channels,
        std::unordered_map<uint32_t, std::unique_ptr<ClientType>>& clients,
        std::function<std::unique_ptr<ClientType>(std::shared_ptr<grpc::ChannelInterface>)> client_factory) {

        if (config_.clients.count(service_name) == 0) {
            LOG_WARN("{} not found in configuration (optional)", service_name);
            return;
        }

        const auto& service_config = config_.clients.at(service_name);

        if (service_config.instances.empty()) {
            LOG_WARN("{} has no instances configured, skipping", service_name);
            return;
        }

        LOG_DEBUG("Initializing {} instance(s) of {}", service_config.instances.size(), service_name);

        for (const auto& instance : service_config.instances) {
            const auto address = instance.server + ":" + std::to_string(instance.port);
            LOG_DEBUG("Creating {} client for instance {} at {}", service_name, instance.id, address);

            auto channel = createChannel(address);
            std::shared_ptr<grpc::ChannelInterface> channel_interface =
                std::static_pointer_cast<grpc::ChannelInterface>(channel);

            channels[instance.id] = channel;
            clients[instance.id] = client_factory(channel_interface);

            LOG_DEBUG("{} instance {} initialized successfully", service_name, instance.id);
        }
    }

    template<typename ClientType>
    void GrpcClientManager::shutdownService(
        const std::string& service_name,
        std::unordered_map<uint32_t, std::shared_ptr<grpc::Channel>>& channels,
        std::unordered_map<uint32_t, std::unique_ptr<ClientType>>& clients) {

        for (const auto& [instance_id, _] : clients) {
            LOG_DEBUG("Closing {} client instance {}", service_name, instance_id);
        }
        clients.clear();

        for (const auto& [instance_id, channel] : channels) {
            if (channel) {
                channel->GetState(true);
                LOG_DEBUG("{} channel {} closed", service_name, instance_id);
            }
        }
        channels.clear();
    }

    std::shared_ptr<grpc::Channel> GrpcClientManager::createChannel(const std::string& address) {
        if (address.empty()) {
            throw std::invalid_argument("Service address cannot be empty");
        }

        LOG_DEBUG("Creating gRPC channel to: {}", address);

        auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());

        if (!channel) {
            throw std::runtime_error("Failed to create gRPC channel to " + address);
        }

        LOG_DEBUG("gRPC channel created successfully");
        return channel;
    }
} // namespace service::infrastructure
