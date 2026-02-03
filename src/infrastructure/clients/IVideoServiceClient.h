#pragma once

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
        virtual Result<void> enableOptionalElement(const std::string& element) = 0;
        virtual Result<void> disableOptionalElement(const std::string& element) = 0;
    };
} // namespace service::infrastructure
