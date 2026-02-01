#include "FakeSimpleCamera.h"

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
        common::types::info info_ = "Fake Simple Camera";
    } // unnamed namespace

    Result<void> FakeSimpleCamera::setZoom(common::types::zoom zoom) const {
        zoom_ = zoom;
        return Result<void>::success();
    }

    Result<common::types::zoom> FakeSimpleCamera::getZoom() const {
        return Result<common::types::zoom>::success(zoom_);
    }

    Result<common::types::info> FakeSimpleCamera::getInfo() const {
        return Result<common::types::info>::success(info_);
    }

    Result<void> FakeSimpleCamera::open() {
        return Result<void>::success();
    }

    Result<void> FakeSimpleCamera::close() {
        return Result<void>::success();
    }

    common::types::ZoomRange FakeSimpleCamera::getZoomLimits() const {
        return zoom_limits_;
    }

    Result<void> FakeSimpleCamera::setFocus(common::types::focus focus) const {
        focus_ = focus;
        return Result<void>::success();
    }

    Result<common::types::focus> FakeSimpleCamera::getFocus() const {
        return Result<common::types::focus>::success(focus_);
    }

    common::types::FocusRange FakeSimpleCamera::getFocusLimits() const {
        return focus_limits_;
    }
} // namespace service::infrastructure
