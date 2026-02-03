#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <grpcpp/grpcpp.h>

#include "common/config/ConfigManager.h"
#include "infrastructure/clients/ICameraServiceClient.h"
#include "infrastructure/clients/IVideoServiceClient.h"

namespace service::infrastructure {
    /**
     * Manages gRPC client connections and lifecycle
     * Creates channels and stubs based on InfrastructureConfig
     * Supports multiple services with multiple instances each
     */
    class GrpcClientManager {
    public:
        explicit GrpcClientManager(const common::InfrastructureConfig& config);
        ~GrpcClientManager();

        GrpcClientManager(const GrpcClientManager&) = delete;
        GrpcClientManager& operator=(const GrpcClientManager&) = delete;

        /**
         * Initialize all clients from configuration
         * Creates gRPC channels for each configured service
         */
        void initialize();

        /**
         * Shutdown all clients
         * Closes all gRPC channels gracefully
         */
        void shutdown();

        /**
         * Get camera service client by instance ID
         * @param instance_id instance ID [0-3]
         * @return pointer to ICameraServiceClient, nullptr if not initialized
         * @throws std::invalid_argument if camera_service not configured
         */
        ICameraServiceClient* getCameraServiceClient(uint32_t instance_id) const;

        /**
         * Get video service client by instance ID
         * @param instance_id instance ID [0-3]
         * @return pointer to IVideoServiceClient, nullptr if not initialized
         * @throws std::invalid_argument if video_service not configured
         */
        IVideoServiceClient* getVideoServiceClient(uint32_t instance_id) const;

    private:
        const common::InfrastructureConfig& config_;

        // Camera service clients
        std::unordered_map<uint32_t, std::shared_ptr<grpc::Channel>> camera_channels_;
        std::unordered_map<uint32_t, std::unique_ptr<ICameraServiceClient>> camera_clients_;

        // Video service clients
        std::unordered_map<uint32_t, std::shared_ptr<grpc::Channel>> video_channels_;
        std::unordered_map<uint32_t, std::unique_ptr<IVideoServiceClient>> video_clients_;

        /**
         * Create a gRPC channel to the specified address
         */
        std::shared_ptr<grpc::Channel> createChannel(const std::string& address);

        /**
         * Initialize service clients from configuration
         * @param service_name Name of the service to initialize
         * @param channels Map to store channels
         * @param clients Map to store clients
         * @param client_factory Function to create client from channel
         */
        template<typename ClientType>
        void initializeService(
            const std::string& service_name,
            std::unordered_map<uint32_t, std::shared_ptr<grpc::Channel>>& channels,
            std::unordered_map<uint32_t, std::unique_ptr<ClientType>>& clients,
            std::function<std::unique_ptr<ClientType>(std::shared_ptr<grpc::ChannelInterface>)> client_factory);

        /**
         * Shutdown service clients
         * @param service_name Name of the service to shutdown
         * @param channels Map of channels to close
         * @param clients Map of clients to clear
         */
        template<typename ClientType>
        void shutdownService(
            const std::string& service_name,
            std::unordered_map<uint32_t, std::shared_ptr<grpc::Channel>>& channels,
            std::unordered_map<uint32_t, std::unique_ptr<ClientType>>& clients);
    };
} // namespace service::infrastructure
