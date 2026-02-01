#pragma once

#include <memory>

#include "common/types/CameraTypes.h"
#include "common/types/Result.h"
#include "infrastructure/camera/hal/ICamera.h"

namespace service::infrastructure {
    class ICameraHw;

    class Camera final : public ICamera {
    public:
        explicit Camera(std::unique_ptr<ICameraHw> camera_strategy);
        ~Camera() override;

        Result<void> setZoom(common::types::zoom normalized_zoom) const override;
        Result<common::types::zoom> getZoom() const override;

        Result<void> setFocus(common::types::focus normalized_focus) const override;
        Result<common::types::focus> getFocus() const override;
        Result<void> enableAutoFocus(bool on) const override;

        Result<common::types::info> getInfo() const override;

        Result<void> stabilize(bool on) const override;

        Result<common::capabilities::CapabilityList> getCapabilities() const override;

        Result<void> open() override;
        Result<void> close() override;
        bool isConnected() const override;

        bool hasZoomCapability() const;
        bool hasFocusCapability() const;

    private:
        std::unique_ptr<ICameraHw> camera_hw_;
        bool connected_ {false};

        static bool isValidNormalizedZoom(common::types::zoom value);
        static bool isValidNormalizedFocus(common::types::focus value);
        bool isValidCameraZoom(common::types::zoom value) const;
        bool isValidCameraFocus(common::types::focus value) const;
        common::types::zoom normalizeZoom(common::types::zoom camera_zoom) const;
        common::types::focus normalizeFocus(common::types::focus camera_focus) const;
        common::types::zoom denormalizeZoom(common::types::zoom normalized_zoom) const;
        common::types::focus denormalizeFocus(common::types::focus normalized_focus) const;

        common::types::ZoomRange getZoomLimits() const override;
        common::types::FocusRange getFocusLimits() const override;

        template <typename Capability>
        Capability* getCapability() const;
    };
} // namespace service::infrastructure