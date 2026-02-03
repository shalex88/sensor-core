#include "RequestHandler.h"

#include "common/logger/Logger.h"
#include "core/ICore.h"

namespace service::api {
    RequestHandler::RequestHandler(std::unique_ptr<core::ICore> core)
        : core_(std::move(core)), running_(false) {
        if (!core_) {
            throw std::invalid_argument("Core cannot be null");
        }
    }

    RequestHandler::~RequestHandler() {
        if (stop().isError()) {
            LOG_ERROR("RequestHandler failed to stop gracefully");
        }
    }

    Result<void> RequestHandler::start() {
        LOG_DEBUG("Starting RequestHandler...");
        if (const auto init_result = core_->start(); init_result.isError()) {
            return Result<void>::error(init_result.error());
        }

        running_ = true;
        LOG_DEBUG("RequestHandler started");
        return Result<void>::success();
    }

    Result<void> RequestHandler::stop() {
        if (!isRunning()) {
            return Result<void>::success();
        }

        LOG_DEBUG("Stopping RequestHandler...");
        running_ = false;

        if (core_) {
            if (const auto shutdown_result = core_->stop(); shutdown_result.isError()) {
                LOG_ERROR("Error stopping core: {}", shutdown_result.error());
                return Result<void>::error("Failed to shut down core: " + shutdown_result.error());
            }
        }
        LOG_DEBUG("RequestHandler stopped");
        return Result<void>::success();
    }

    bool RequestHandler::isRunning() const {
        return running_;
    }

    Result<void> RequestHandler::setZoom(uint32_t camera_id, const common::types::zoom zoom_level) const {
        if (!isRunning()) {
            return Result<void>::error("RequestHandler is not running");
        }

        LOG_INFO("Request: {} camera_id={} zoom={}", __func__, camera_id, zoom_level);

        auto operation = core_->setZoom(camera_id, zoom_level);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<common::types::zoom> RequestHandler::getZoom(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<common::types::zoom>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} camera_id={}", __func__, camera_id);

        auto operation = core_->getZoom(camera_id);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: {}", operation.value());
        }

        return operation;
    }

    Result<void> RequestHandler::goToMinZoom(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} camera_id={}", __func__, camera_id);

        auto operation = core_->goToMinZoom(camera_id);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<void> RequestHandler::goToMaxZoom(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} camera_id={}", __func__, camera_id);

        auto operation = core_->goToMaxZoom(camera_id);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<void> RequestHandler::setFocus(uint32_t camera_id, const common::types::focus focus_value) const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} camera_id={} focus={}", __func__, camera_id, focus_value);

        auto operation = core_->setFocus(camera_id, focus_value);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<common::types::focus> RequestHandler::getFocus(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<common::types::focus>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} camera_id={}", __func__, camera_id);

        auto operation = core_->getFocus(camera_id);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: {}", operation.value());
        }

        return operation;
    }

    Result<void> RequestHandler::enableAutoFocus(uint32_t camera_id, bool on) const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} camera_id={} enable={}", __func__, camera_id, on);

        auto operation = core_->enableAutoFocus(camera_id, on);
        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<common::types::info> RequestHandler::getInfo(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<common::types::info>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} camera_id={}", __func__, camera_id);

        auto operation = core_->getInfo(camera_id);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: {}", operation.value());
        }

        return operation;
    }

    Result<void> RequestHandler::stabilize(uint32_t camera_id, const bool on) const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} camera_id={} enable={}", __func__, camera_id, on);

        auto operation = core_->stabilize(camera_id, on);
        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<common::capabilities::CapabilityList> RequestHandler::getCapabilities(uint32_t camera_id) const {
        if (!isRunning()) {
            return Result<common::capabilities::CapabilityList>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} camera_id={}", __func__, camera_id);

        auto operation = core_->getCapabilities(camera_id);
        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: {} capabilities", operation.value().size());
        }

        return operation;
    }

    Result<void> RequestHandler::enableOptionalElement(uint32_t camera_id, const std::string& element) const {
        if (!isRunning()) {
            return Result<void>::error("RequestHandler is not running");
        }

        LOG_INFO("Request: {} camera_id={} element={}", __func__, camera_id, element);

        auto operation = core_->enableOptionalElement(camera_id, element);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<void> RequestHandler::disableOptionalElement(uint32_t camera_id, const std::string& element) const {
        if (!isRunning()) {
            return Result<void>::error("RequestHandler is not running");
        }

        LOG_INFO("Request: {} camera_id={} element={}", __func__, camera_id, element);

        auto operation = core_->disableOptionalElement(camera_id, element);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }
} // namespace service::api