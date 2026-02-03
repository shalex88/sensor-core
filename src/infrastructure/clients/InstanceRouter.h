#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "common/logger/Logger.h"

namespace service::infrastructure {
    /**
     * Generic instance routing helper for multi-instance services
     * Routes requests to the correct client instance by ID
     * Provides consistent error handling across all services
     */
    template<typename ClientType>
    class InstanceRouter {
    public:
        /**
         * Get client for a specific instance ID
         * @param service_name Name of the service (for logging)
         * @param instance_id Instance ID to retrieve
         * @param clients Map of instance_id -> client
         * @return Pointer to client, nullptr if not initialized
         * @throws std::invalid_argument if instance_id not in clients
         */
        static ClientType* getClient(
            const std::string& service_name,
            uint32_t instance_id,
            const std::unordered_map<uint32_t, std::unique_ptr<ClientType>>& clients) {

            auto it = clients.find(instance_id);
            if (it == clients.end()) {
                LOG_WARN("{} client for instance {} not initialized", service_name, instance_id);
                return nullptr;
            }

            return it->second.get();
        }

        /**
         * Check if service is configured
         * @param service_name Name of the service
         * @param clients Map of instance_id -> client
         * @return true if at least one instance is configured
         */
        static bool isConfigured(
            const std::unordered_map<uint32_t, std::unique_ptr<ClientType>>& clients) {
            return !clients.empty();
        }
    };
} // namespace service::infrastructure
