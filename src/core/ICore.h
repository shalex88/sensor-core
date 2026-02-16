#pragma once

#include <string>
#include <vector>

#include "common/types/CameraTypes.h"
#include "common/types/CameraCapabilities.h"
#include "common/types/Result.h"

namespace service::core {
    class ICore {
    public:
        virtual ~ICore() = default;

        virtual Result<void> start() = 0;
        virtual Result<void> stop() = 0;

        // Business methods for zoom operations
        virtual Result<void> setZoom(uint32_t camera_id, common::types::zoom zoom_level) const = 0;
        virtual Result<common::types::zoom> getZoom(uint32_t camera_id) const = 0;
        virtual Result<void> goToMinZoom(uint32_t camera_id) const = 0;
        virtual Result<void> goToMaxZoom(uint32_t camera_id) const = 0;

        // Business methods for focus operations
        virtual Result<void> setFocus(uint32_t camera_id, common::types::focus focus_value) const = 0;
        virtual Result<common::types::focus> getFocus(uint32_t camera_id) const = 0;
        virtual Result<void> enableAutoFocus(uint32_t camera_id, bool on) const = 0;
        virtual Result<bool> getAutoFocus(uint32_t camera_id) const = 0;

        // Business methods for info operations
        virtual Result<common::types::info> getInfo(uint32_t camera_id) const = 0;

        // Business methods for advanced operations
        virtual Result<void> stabilize(uint32_t camera_id, bool on) const = 0;
        virtual Result<bool> getStabilization(uint32_t camera_id) const = 0;

        // Capability inquiry
        virtual Result<common::capabilities::CapabilityList> getCapabilities(uint32_t camera_id) const = 0;

        // Video operations (routed by camera_id)
        virtual Result<void> SetVideoCapabilityState(
            uint32_t camera_id,
            const std::string& capability,
            bool enable) const = 0;
        virtual Result<std::vector<std::string>> getVideoCapabilities(uint32_t camera_id) const = 0;
        virtual Result<bool> getVideoCapabilityState(uint32_t camera_id, const std::string& capability) const = 0;
    };
} // namespace service::core
