#pragma once

#include "common/types/CameraCapabilities.h"
#include "common/types/Result.h"

namespace service::infrastructure {
    class ICamera : public common::capabilities::IZoomCapable,
                       public common::capabilities::IFocusCapable,
                       public common::capabilities::IAutoFocusCapable,
                       public common::capabilities::IStabilizeCapable,
                       public common::capabilities::IInfoCapable {
    public:
        ~ICamera() override = default;

        virtual Result<void> open() = 0;
        virtual Result<void> close() = 0;
        virtual bool isConnected() const = 0;
        virtual Result<common::capabilities::CapabilityList> getCapabilities() const = 0;
    };
} // namespace service::infrastructure