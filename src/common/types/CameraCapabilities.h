#pragma once

#include <vector>

#include "common/types/CameraTypes.h"
#include "common/types/Result.h"

namespace service::common::capabilities {
    enum class Capability {
        Zoom,
        Focus,
        AutoFocus,
        Info,
        Stabilization
    };

    using CapabilityList = std::vector<Capability>;

    class IZoomCapable {
    public:
        virtual ~IZoomCapable() = default;

        virtual Result<void> setZoom(types::zoom zoom_level) const = 0;
        virtual Result<types::zoom> getZoom() const = 0;
        virtual types::ZoomRange getZoomLimits() const = 0;
    };

    class IFocusCapable {
    public:
        virtual ~IFocusCapable() = default;

        virtual Result<void> setFocus(types::focus focus_value) const = 0;
        virtual Result<types::focus> getFocus() const = 0;
        virtual types::FocusRange getFocusLimits() const = 0;
    };

    class IAutoFocusCapable {
    public:
        virtual ~IAutoFocusCapable() = default;

        virtual Result<void> enableAutoFocus(bool on) const = 0;
    };

    class IInfoCapable {
    public:
        virtual ~IInfoCapable() = default;

        virtual Result<types::info> getInfo() const = 0;
    };

    class IStabilizeCapable {
    public:
        virtual ~IStabilizeCapable() = default;

        virtual Result<void> stabilize(bool on) const = 0;
    };
} // namespace service::common::capabilities