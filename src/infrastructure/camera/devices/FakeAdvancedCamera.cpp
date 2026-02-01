#include "FakeAdvancedCamera.h"

namespace service::infrastructure {
    namespace {
        constexpr common::types::ZoomRange zoom_limits_{
            .min = 0x0,
            .max = 0xFF
        };

        constexpr common::types::FocusRange focus_limits_{
            .min = 0x0,
            .max = 0xFF
        };

        common::types::zoom zoom_ = zoom_limits_.min;
        common::types::focus focus_ = focus_limits_.min;
        bool auto_focus_enabled_ = true;
        bool stabilize_enabled_ = false;
        common::types::info info_ = "Fake Advanced Camera";
    } // unnamed namespace

    Result<void> FakeAdvancedCamera::setZoom(const common::types::zoom zoom) const {
        zoom_ = zoom;
        return Result<void>::success();
    }

    Result<common::types::zoom> FakeAdvancedCamera::getZoom() const {
        return Result<common::types::zoom>::success(zoom_);
    }

    Result<void> FakeAdvancedCamera::setFocus(common::types::focus focus) const {
        if (auto_focus_enabled_) {
            return Result<void>::error("Cannot set focus value while autofocus is enabled");
        }
        focus_ = focus;
        return Result<void>::success();
    }

    Result<common::types::focus> FakeAdvancedCamera::getFocus() const {
        if (auto_focus_enabled_) {
            return Result<common::types::focus>::error("Cannot get focus value while autofocus is enabled");
        }
        return Result<common::types::focus>::success(focus_);
    }

    Result<common::types::info> FakeAdvancedCamera::getInfo() const {
        return Result<common::types::info>::success(info_);
    }

    Result<void> FakeAdvancedCamera::open() {
        return Result<void>::success();
    }

    Result<void> FakeAdvancedCamera::close() {
        return Result<void>::success();
    }

    Result<void> FakeAdvancedCamera::enableAutoFocus(bool enable) const {
        auto_focus_enabled_ = enable;
        return Result<void>::success();
    }

    Result<bool> FakeAdvancedCamera::isAutoFocusEnabled() const {
        return Result<bool>::success(auto_focus_enabled_);
    }

    Result<void> FakeAdvancedCamera::stabilize(bool enable) const {
        stabilize_enabled_ = enable;
        return Result<void>::success();
    }

    common::types::ZoomRange FakeAdvancedCamera::getZoomLimits() const {
        return zoom_limits_;
    }

    common::types::FocusRange FakeAdvancedCamera::getFocusLimits() const {
        return focus_limits_;
    }
} // namespace service::infrastructure
