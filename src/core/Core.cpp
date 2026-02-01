#include "Core.h"

#include "common/logger/Logger.h"
#include "infrastructure/camera/hal/ICamera.h"

namespace service::core {
    Core::Core(std::unique_ptr<infrastructure::ICamera> camera)
        : camera_(std::move(camera)), is_running_(false) {
        if (!camera_) {
            throw std::invalid_argument("Cannot initialize Core with null camera");
        }
    }

    Core::~Core() {
        if (stop().isError()) {
            LOG_ERROR("Failed to shut down Core properly");
        }
    }

    Result<void> Core::start() {
        LOG_DEBUG("Starting...");

        if (!camera_->isConnected()) {
            if (const auto connect_result = camera_->open(); connect_result.isError()) {
                return Result<void>::error(connect_result.error());
            }
        }

        is_running_ = true;
        LOG_DEBUG("Running");
        return Result<void>::success();
    }

    Result<void> Core::stop() {
        if (!isRunning()) {
            return Result<void>::success();
        }

        is_running_ = false;

        LOG_DEBUG("Stopping...");

        if (camera_ && camera_->isConnected()) {
            if (const auto disconnect_result = camera_->close(); disconnect_result.isError()) {
                return Result<void>::error(disconnect_result.error());
            }
        }

        LOG_DEBUG("Stopped");
        return Result<void>::success();
    }

    bool Core::isRunning() const {
        return is_running_;
    }

    Result<void> Core::setZoom(const common::types::zoom zoom_level) const {
        if (!isRunning()) {
            return Result<void>::error("Core is not initialized");
        }

        return camera_->setZoom(zoom_level);
    }

    Result<common::types::zoom> Core::getZoom() const {
        if (!isRunning()) {
            return Result<common::types::zoom>::error("Core is not initialized");
        }

        return camera_->getZoom();
    }

    Result<void> Core::goToMinZoom() const {
        return setZoom(common::types::MIN_NORMALIZED_ZOOM);
    }

    Result<void> Core::goToMaxZoom() const {
        return setZoom(common::types::MAX_NORMALIZED_ZOOM);
    }

    Result<void> Core::setFocus(const common::types::focus focus_value) const {
        if (!isRunning()) {
            return Result<void>::error("Core is not initialized");
        }

        return camera_->setFocus(focus_value);
    }

    Result<common::types::focus> Core::getFocus() const {
        if (!isRunning()) {
            return Result<common::types::focus>::error("Core is not initialized");
        }

        return camera_->getFocus();
    }

    Result<void> Core::enableAutoFocus(const bool on) const {
        if (!isRunning()) {
            return Result<void>::error("Core is not initialized");
        }

        return camera_->enableAutoFocus(on);
    }

    Result<common::types::info> Core::getInfo() const {
        if (!isRunning()) {
            return Result<common::types::info>::error("Core is not initialized");
        }

        return camera_->getInfo();
    }

    Result<void> Core::stabilize(const bool on) const {
        if (!isRunning()) {
            return Result<void>::error("Core is not initialized");
        }

        return camera_->stabilize(on);
    }

    Result<common::capabilities::CapabilityList> Core::getCapabilities() const {
        if (!isRunning()) {
            return Result<common::capabilities::CapabilityList>::error("Core is not initialized");
        }

        return camera_->getCapabilities();
    }
} // namespace service::core
