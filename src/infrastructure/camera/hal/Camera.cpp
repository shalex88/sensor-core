#include "Camera.h"

#include "common/logger/Logger.h"
#include "infrastructure/camera/hal/ICameraHw.h"

namespace service::infrastructure {
    Camera::Camera(std::unique_ptr<ICameraHw> camera_strategy)
        : camera_hw_(std::move(camera_strategy)) {
        if (!camera_hw_) {
            throw std::invalid_argument("Camera hardware cannot be null");
        }
        if (hasZoomCapability()) {
            if (const auto [min, max] = getZoomLimits(); min >= max) {
                throw std::runtime_error("Invalid zoom limits from camera");
            }
        }
        if (hasFocusCapability()) {
            if (const auto [min, max] = getFocusLimits(); min >= max) {
                throw std::runtime_error("Invalid focus limits from camera");
            }
        }
    }

    Camera::~Camera() {
        if (close().isError()) {
            LOG_ERROR("Failed to disconnect Camera");
        }
    }

    Result<void> Camera::setZoom(const common::types::zoom normalized_zoom) const {
        if (!isConnected()) {
            return Result<void>::error("Camera not connected");
        }

        const auto* zoom_capable_camera = getCapability<common::capabilities::IZoomCapable>();
        if (!zoom_capable_camera) {
            return Result<void>::error("Camera doesn't support zoom");
        }

        if (!isValidNormalizedZoom(normalized_zoom)) {
            return Result<void>::error("Invalid normalized zoom value. Must be 0-100");
        }

        const common::types::zoom camera_zoom = denormalizeZoom(normalized_zoom);
        if (!isValidCameraZoom(camera_zoom)) {
            return Result<void>::error("Invalid zoom value");
        }

        LOG_DEBUG("{} normalized: {}, converted: {}", __func__, normalized_zoom, camera_zoom);
        return zoom_capable_camera->setZoom(camera_zoom);
    }

    Result<common::types::zoom> Camera::getZoom() const {
        if (!isConnected()) {
            return Result<common::types::zoom>::error("Camera not connected");
        }

        const auto* zoom_capable_camera = getCapability<common::capabilities::IZoomCapable>();
        if (!zoom_capable_camera) {
            return Result<common::types::zoom>::error("Camera doesn't support zoom");
        }

        LOG_DEBUG(__func__);
        const auto zoom_result = zoom_capable_camera->getZoom();

        if (zoom_result.isError()) {
            return Result<common::types::zoom>::error(zoom_result.error());
        }

        const common::types::zoom camera_zoom = zoom_result.value();

        if (!isValidCameraZoom(camera_zoom)) {
            return Result<common::types::zoom>::error("Invalid zoom value from camera");
        }

        const common::types::zoom normalized_zoom = normalizeZoom(camera_zoom);

        LOG_DEBUG("{} camera: {}, normalized: {}", __func__, camera_zoom, normalized_zoom);
        return Result<common::types::zoom>::success(normalized_zoom);
    }

    Result<void> Camera::setFocus(const common::types::focus normalized_focus) const {
        if (!isConnected()) {
            return Result<void>::error("Camera not connected");
        }

        const auto* focus_capable = getCapability<common::capabilities::IFocusCapable>();
        if (!focus_capable) {
            return Result<void>::error("Camera doesn't support focus");
        }

        if (!isValidNormalizedFocus(normalized_focus)) {
            return Result<void>::error("Invalid normalized focus value. Must be 0-100");
        }

        const common::types::focus camera_focus = denormalizeFocus(normalized_focus);
        if (!isValidCameraFocus(camera_focus)) {
            return Result<void>::error("Invalid focus value");
        }

        LOG_DEBUG("{} normalized: {}, converted: {}", __func__, normalized_focus, camera_focus);
        return focus_capable->setFocus(camera_focus);
    }

    Result<common::types::focus> Camera::getFocus() const {
        if (!isConnected()) {
            return Result<common::types::focus>::error("Camera not connected");
        }

        const auto* focus_capable = getCapability<common::capabilities::IFocusCapable>();
        if (!focus_capable) {
            return Result<common::types::focus>::error("Camera doesn't support focus");
        }

        LOG_DEBUG(__func__);
        const auto focus_result = focus_capable->getFocus();

        if (focus_result.isError()) {
            return Result<common::types::focus>::error(focus_result.error());
        }

        const common::types::focus camera_focus = focus_result.value();

        if (!isValidCameraFocus(camera_focus)) {
            return Result<common::types::focus>::error("Invalid focus value from camera");
        }

        const common::types::focus normalized_focus = normalizeFocus(camera_focus);

        LOG_DEBUG("{} camera: {}, normalized: {}", __func__, camera_focus, normalized_focus);
        return Result<common::types::focus>::success(normalized_focus);
    }

    Result<void> Camera::enableAutoFocus(const bool on) const {
        if (!isConnected()) {
            return Result<void>::error("Camera not connected");
        }

        const auto* auto_focus_capable = getCapability<common::capabilities::IAutoFocusCapable>();
        if (!auto_focus_capable) {
            return Result<void>::error("Camera doesn't support autofocus");
        }

        LOG_DEBUG(__func__);

        return auto_focus_capable->enableAutoFocus(on);
    }

    Result<common::types::info> Camera::getInfo() const {
        if (!isConnected()) {
            return Result<common::types::info>::error("Camera not connected");
        }

        const auto* info_capable = getCapability<common::capabilities::IInfoCapable>();
        if (!info_capable) {
            return Result<common::types::info>::error("Camera doesn't support info");
        }

        auto info_result = info_capable->getInfo();
        if (info_result.isError()) {
            return Result<common::types::info>::error(info_result.error());
        }

        LOG_DEBUG("{} {}", __func__, info_result.value());
        return info_result;
    }

    Result<void> Camera::stabilize(const bool on) const {
        if (!isConnected()) {
            return Result<void>::error("Camera not connected");
        }

        const auto* stabilize_capable = getCapability<common::capabilities::IStabilizeCapable>();
        if (!stabilize_capable) {
            return Result<void>::error("Camera doesn't support stabilization");
        }

        LOG_DEBUG(__func__);

        return stabilize_capable->stabilize(on);
    }

    Result<common::capabilities::CapabilityList> Camera::getCapabilities() const {
        if (!isConnected()) {
            return Result<common::capabilities::CapabilityList>::error("Camera not connected");
        }

        common::capabilities::CapabilityList capabilities;

        if (hasZoomCapability()) {
            capabilities.emplace_back(common::capabilities::Capability::Zoom);
        }

        if (hasFocusCapability()) {
            capabilities.emplace_back(common::capabilities::Capability::Focus);
        }

        if (getCapability<common::capabilities::IAutoFocusCapable>() != nullptr) {
            capabilities.emplace_back(common::capabilities::Capability::AutoFocus);
        }

        if (getCapability<common::capabilities::IInfoCapable>() != nullptr) {
            capabilities.emplace_back(common::capabilities::Capability::Info);
        }

        if (getCapability<common::capabilities::IStabilizeCapable>() != nullptr) {
            capabilities.emplace_back(common::capabilities::Capability::Stabilization);
        }

        return Result<common::capabilities::CapabilityList>::success(capabilities);
    }

    Result<void> Camera::open() {
        if (connected_) {
            return Result<void>::error("Camera already connected");
        }

        LOG_DEBUG("Connecting camera...");
        if (const auto connect_result = camera_hw_->open(); connect_result.isError()) {
            return Result<void>::error(connect_result.error());
        }

        connected_ = true;
        LOG_DEBUG("Camera connected");
        return Result<void>::success();
    }

    Result<void> Camera::close() {
        if (!connected_) {
            return Result<void>::success();
        }

        LOG_DEBUG("Disconnecting camera...");
        connected_ = false;

        if (const auto disconnect_result = camera_hw_->close(); disconnect_result.isError()) {
            return Result<void>::error(disconnect_result.error());
        }

        LOG_DEBUG("Camera disconnected");
        return Result<void>::success();
    }

    bool Camera::isConnected() const {
        return connected_;
    }

    bool Camera::hasZoomCapability() const {
        return getCapability<common::capabilities::IZoomCapable>() != nullptr;
    }

    bool Camera::hasFocusCapability() const {
        return getCapability<common::capabilities::IFocusCapable>() != nullptr;
    }

    common::types::ZoomRange Camera::getZoomLimits() const {
        const auto* zoom_capable_camera = getCapability<common::capabilities::IZoomCapable>();
        if (!zoom_capable_camera) {
            return {0, 0};
        }
        return zoom_capable_camera->getZoomLimits();
    }

    common::types::FocusRange Camera::getFocusLimits() const {
        const auto* focus_capable_camera = getCapability<common::capabilities::IFocusCapable>();
        if (!focus_capable_camera) {
            return {0, 0};
        }
        return focus_capable_camera->getFocusLimits();
    }

    bool Camera::isValidCameraZoom(const common::types::zoom value) const {
        return value >= getZoomLimits().min && value <= getZoomLimits().max;
    }

    bool Camera::isValidCameraFocus(const common::types::focus value) const {
        return value >= getFocusLimits().min && value <= getFocusLimits().max;
    }

    bool Camera::isValidNormalizedZoom(const common::types::zoom value) {
        return value >= common::types::MIN_NORMALIZED_ZOOM && value <= common::types::MAX_NORMALIZED_ZOOM;
    }

    bool Camera::isValidNormalizedFocus(const common::types::focus value) {
        return value >= common::types::MIN_NORMALIZED_FOCUS && value <= common::types::MAX_NORMALIZED_FOCUS;
    }

    common::types::zoom Camera::normalizeZoom(const common::types::zoom camera_zoom) const {
        const auto [min, max] = getZoomLimits();
        const auto range = max - min;

        const long long numerator = static_cast<long long>(camera_zoom - min) * common::types::MAX_NORMALIZED_ZOOM;
        const long long rounded = (numerator >= 0)
            ? (numerator + range / 2) / range
            : -(((-numerator) + range / 2) / range);

        return static_cast<common::types::zoom>(rounded);
    }

    common::types::focus Camera::normalizeFocus(const common::types::focus camera_focus) const {
        const auto [min, max] = getFocusLimits();
        const auto range = max - min;

        const long long numerator = static_cast<long long>(camera_focus - min) * common::types::MAX_NORMALIZED_FOCUS;
        const long long rounded = (numerator >= 0)
            ? (numerator + range / 2) / range
            : -(((-numerator) + range / 2) / range);

        return static_cast<common::types::focus>(rounded);
    }

    common::types::zoom Camera::denormalizeZoom(const common::types::zoom normalized_zoom) const {
        const auto [min, max] = getZoomLimits();
        const auto range = max - min;

        const long long numerator = static_cast<long long>(normalized_zoom) * range;
        const long long rounded = (numerator >= 0)
            ? (numerator + common::types::MAX_NORMALIZED_ZOOM / 2) / common::types::MAX_NORMALIZED_ZOOM
            : -(((-numerator) + common::types::MAX_NORMALIZED_ZOOM / 2) / common::types::MAX_NORMALIZED_ZOOM);

        return static_cast<common::types::zoom>(min + rounded);
    }

    common::types::focus Camera::denormalizeFocus(const common::types::focus normalized_focus) const {
        const auto [min, max] = getFocusLimits();
        const auto range = max - min;

        const long long numerator = static_cast<long long>(normalized_focus) * range;
        const long long rounded = (numerator >= 0)
            ? (numerator + common::types::MAX_NORMALIZED_FOCUS / 2) / common::types::MAX_NORMALIZED_FOCUS
            : -(((-numerator) + common::types::MAX_NORMALIZED_FOCUS / 2) / common::types::MAX_NORMALIZED_FOCUS);

        return static_cast<common::types::focus>(min + rounded);
    }

    template <typename Capability>
    Capability* Camera::getCapability() const {
        return dynamic_cast<Capability*>(camera_hw_.get());
    }
} // namespace service::infrastructure