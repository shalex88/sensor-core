#include "MwirCamera.h"

#include "infrastructure/camera/protocol/itl/ItlProtocol.h"
#include "infrastructure/camera/transport/ethernet/MwirOpcodes.h"

namespace service::infrastructure {
    namespace {
        constexpr common::types::ZoomRange zoom_limits_{ //TODO: define real limits
            .min = 0x0,
            .max = 0xFF
        };

        constexpr common::types::FocusRange focus_limits_{ //TODO: define real limits
            .min = 0x0,
            .max = 0xFF
        };

        common::types::zoom zoom_ = zoom_limits_.min;
        common::types::focus focus_ = focus_limits_.min;
        bool auto_focus_enabled_ = true;
    } // unnamed namespace

    MwirCamera::MwirCamera(std::unique_ptr<ItlProtocol> protocol) : protocol_(std::move(protocol)) {
        if (!protocol_) {
            throw std::invalid_argument("Protocol cannot be null");
        }
    }

    MwirCamera::~MwirCamera() = default;

    Result<void> MwirCamera::setZoom(common::types::zoom zoom) const {
        zoom_ = zoom;
        return Result<void>::success();
    }

    Result<common::types::zoom> MwirCamera::getZoom() const {
        return Result<common::types::zoom>::success(zoom_);
    }

    Result<void> MwirCamera::setFocus(common::types::focus focus) const {
        if (auto_focus_enabled_) {
            return Result<void>::error("Cannot set focus value while autofocus is enabled");
        }
        focus_ = focus;
        return Result<void>::success();
    }

    Result<common::types::focus> MwirCamera::getFocus() const {
        if (auto_focus_enabled_) {
            return Result<common::types::focus>::error("Cannot get focus value while autofocus is enabled");
        }
        return Result<common::types::focus>::success(focus_);
    }

    Result<common::types::info> MwirCamera::getInfo() const {
        std::vector<std::byte> payload;

        const auto info = protocol_->sendPayload(MWIR_GET_VERSION, std::span<const std::byte>{payload});
        if (info.isError()) {
            return Result<common::types::info>::error(info.error());
        }
        const std::string result = "v" + std::to_string(static_cast<uint8_t>(info.value().at(0))) + "." +
                                  std::to_string(static_cast<uint8_t>(info.value().at(1))) + "." +
                                  std::to_string(static_cast<uint8_t>(info.value().at(2))) + "." +
                                  std::to_string(static_cast<uint8_t>(info.value().at(3)));
        return Result<common::types::info>::success(result);
    }

    Result<void> MwirCamera::open() {
        if (const auto result = protocol_->open(); result.isError()) {
            return Result<void>::error(result.error());
        }
        return Result<void>::success();
    }

    Result<void> MwirCamera::close() {
        if (const auto result = protocol_->close(); !result.isError()) {
            return Result<void>::success();
        }
        return Result<void>::error("Failed to disconnect");
    }

    Result<void> MwirCamera::enableAutoFocus(const bool on) const {
        auto_focus_enabled_ = on;
        return Result<void>::success();
    }

    Result<bool> MwirCamera::isAutoFocusEnabled() const {
        return Result<bool>::success(auto_focus_enabled_);
    }

    common::types::ZoomRange MwirCamera::getZoomLimits() const {
        return zoom_limits_;
    }

    common::types::FocusRange MwirCamera::getFocusLimits() const {
        return focus_limits_;
    }
} // namespace service::infrastructure
