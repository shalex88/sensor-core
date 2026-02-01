#pragma once

#include <string>

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
        virtual Result<void> setZoom(common::types::zoom zoom_level) const = 0;
        virtual Result<common::types::zoom> getZoom() const = 0;
        virtual Result<void> goToMinZoom() const = 0;
        virtual Result<void> goToMaxZoom() const = 0;

        // Business methods for focus operations
        virtual Result<void> setFocus(common::types::focus focus_value) const = 0;
        virtual Result<common::types::focus> getFocus() const = 0;
        virtual Result<void> enableAutoFocus(bool on) const = 0;

        // Business methods for info operations
        virtual Result<common::types::info> getInfo() const = 0;

        // Business methods for advanced operations
        virtual Result<void> stabilize(bool on) const = 0;

        // Capability inquiry
        virtual Result<common::capabilities::CapabilityList> getCapabilities() const = 0;
    };
} // namespace service::core
