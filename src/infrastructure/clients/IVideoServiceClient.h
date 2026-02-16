#pragma once

#include <vector>

#include "common/types/Result.h"

namespace service::infrastructure {
    /**
     * Abstraction over gRPC video_service client
     * Wraps the gRPC stub and converts responses to domain types
     */
    class IVideoServiceClient {
    public:
        virtual ~IVideoServiceClient() = default;

        // Video operations
        virtual Result<void> SetVideoCapabilityState(const std::string& capability, bool enable) = 0;
        virtual Result<bool> getVideoCapabilityState(const std::string& capability) = 0;
        virtual Result<std::vector<std::string>> getVideoCapabilities() = 0;
    };
} // namespace service::infrastructure
