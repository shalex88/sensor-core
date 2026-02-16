#pragma once

#include <string>
#include <vector>
#include "common/types/Result.h"
#include "common/types/CameraTypes.h"
#include "common/types/CameraCapabilities.h"

namespace service::api {
    class IRequestHandler {
    public:
        virtual ~IRequestHandler() = default;

        virtual Result<void> start() = 0;
        virtual Result<void> stop() = 0;
        virtual bool isRunning() const = 0;

        virtual Result<void> setZoom(uint32_t camera_id, common::types::zoom zoom_level) const = 0;
        virtual Result<common::types::zoom> getZoom(uint32_t camera_id) const = 0;
        virtual Result<void> goToMinZoom(uint32_t camera_id) const = 0;
        virtual Result<void> goToMaxZoom(uint32_t camera_id) const = 0;

        virtual Result<void> setFocus(uint32_t camera_id, common::types::focus focus_value) const = 0;
        virtual Result<common::types::focus> getFocus(uint32_t camera_id) const = 0;
        virtual Result<void> enableAutoFocus(uint32_t camera_id, bool on) const = 0;
        virtual Result<bool> getAutoFocus(uint32_t camera_id) const = 0;

        virtual Result<common::types::info> getInfo(uint32_t camera_id) const = 0;

        virtual Result<void> stabilize(uint32_t camera_id, bool on) const = 0;
        virtual Result<bool> getStabilization(uint32_t camera_id) const = 0;

        virtual Result<common::capabilities::CapabilityList> getCapabilities(uint32_t camera_id) const = 0;

        // Video operations (routed by camera_id)
        virtual Result<void> SetVideoCapabilityState(
            uint32_t camera_id,
            const std::string& capability,
            bool enable) const = 0;
        virtual Result<std::vector<std::string>> getVideoCapabilities(uint32_t camera_id) const = 0;
        virtual Result<bool> getVideoCapabilityState(uint32_t camera_id, const std::string& capability) const = 0;
    };
}
