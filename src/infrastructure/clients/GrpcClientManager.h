#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <grpcpp/grpcpp.h>

#include "common/config/ConfigManager.h"
#include "infrastructure/clients/ICameraServiceClient.h"

namespace service::infrastructure {
    /**
     * Manages gRPC client connections and lifecycle
     * Creates channels and stubs based on InfrastructureConfig
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
         * Get camera service client by camera ID
         * @param camera_id camera instance ID [0-3]
         * @return pointer to ICameraServiceClient, nullptr if not initialized or invalid ID
         * @throws std::invalid_argument if camera_id is out of valid range
         */
        ICameraServiceClient* getCameraServiceClient(uint32_t camera_id) const;

    private:
        const common::InfrastructureConfig& config_;
        std::unordered_map<uint32_t, std::shared_ptr<grpc::Channel>> camera_channels_;
        std::unordered_map<uint32_t, std::unique_ptr<ICameraServiceClient>> camera_clients_;

        /**
         * Create a gRPC channel to the specified address
         */
        std::shared_ptr<grpc::Channel> createChannel(const std::string& address);
    };
} // namespace service::infrastructure
