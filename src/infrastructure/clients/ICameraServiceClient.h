#pragma once

#include "common/types/CameraTypes.h"
#include "common/types/CameraCapabilities.h"
#include "common/types/Result.h"

namespace service::infrastructure {
    /**
     * Abstraction over gRPC camera_service client
     * Wraps the gRPC stub and converts responses to domain types
     */
    class ICameraServiceClient {
    public:
        virtual ~ICameraServiceClient() = default;

        // Zoom operations
        virtual Result<void> setZoom(common::types::zoom zoom_level) = 0;
        virtual Result<common::types::zoom> getZoom() = 0;
        virtual Result<void> goToMinZoom() = 0;
        virtual Result<void> goToMaxZoom() = 0;

        // Focus operations
        virtual Result<void> setFocus(common::types::focus focus_value) = 0;
        virtual Result<common::types::focus> getFocus() = 0;
        virtual Result<void> enableAutoFocus(bool on) = 0;

        // Device info
        virtual Result<common::types::info> getInfo() = 0;

        // Advanced operations
        virtual Result<void> stabilize(bool on) = 0;

        // Capabilities
        virtual Result<common::capabilities::CapabilityList> getCapabilities() = 0;
    };
} // namespace service::infrastructure
