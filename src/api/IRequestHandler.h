#pragma once

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

        virtual Result<void> setZoom(common::types::zoom zoom_level) const = 0;
        virtual Result<common::types::zoom> getZoom() const = 0;
        virtual Result<void> goToMinZoom() const = 0;
        virtual Result<void> goToMaxZoom() const = 0;

        virtual Result<void> setFocus(common::types::focus focus_value) const = 0;
        virtual Result<common::types::focus> getFocus() const = 0;
        virtual Result<void> enableAutoFocus(bool on) const = 0;

        virtual Result<common::types::info> getInfo() const = 0;

        virtual Result<void> stabilize(bool on) const = 0;

        virtual Result<common::capabilities::CapabilityList> getCapabilities() const = 0;
    };
}
