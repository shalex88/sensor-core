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

    Result<void> RequestHandler::setZoom(const common::types::zoom zoom_level) const {
        if (!isRunning()) {
            return Result<void>::error("RequestHandler is not running");
        }

        LOG_INFO("Request: {} {}", __func__, zoom_level);

        auto operation = core_->setZoom(zoom_level);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<common::types::zoom> RequestHandler::getZoom() const {
        if (!isRunning()) {
            return Result<common::types::zoom>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {}", __func__);

        auto operation = core_->getZoom();

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: {}", operation.value());
        }

        return operation;
    }

    Result<void> RequestHandler::goToMinZoom() const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {}", __func__);

        auto operation = core_->goToMinZoom();

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<void> RequestHandler::goToMaxZoom() const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {}", __func__);

        auto operation = core_->goToMaxZoom();

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<void> RequestHandler::setFocus(const common::types::focus focus_value) const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} {}", __func__, focus_value);

        auto operation = core_->setFocus(focus_value);

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<common::types::focus> RequestHandler::getFocus() const {
        if (!isRunning()) {
            return Result<common::types::focus>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {}", __func__);

        auto operation = core_->getFocus();

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: {}", operation.value());
        }

        return operation;
    }

    Result<void> RequestHandler::enableAutoFocus(bool on) const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} {}", __func__, on);

        auto operation = core_->enableAutoFocus(on);
        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<common::types::info> RequestHandler::getInfo() const {
        if (!isRunning()) {
            return Result<common::types::info>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {}", __func__);

        auto operation = core_->getInfo();

        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: {}", operation.value());
        }

        return operation;
    }

    Result<void> RequestHandler::stabilize(const bool on) const {
        if (!isRunning()) {
            return Result<void>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {} {}", __func__, on);

        auto operation = core_->stabilize(on);
        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: Success");
        }

        return operation;
    }

    Result<common::capabilities::CapabilityList> RequestHandler::getCapabilities() const {
        if (!isRunning()) {
            return Result<common::capabilities::CapabilityList>::error("Request Handler is not running");
        }

        LOG_INFO("Request: {}", __func__);

        auto operation = core_->getCapabilities();
        if (operation.isError()) {
            LOG_ERROR("Response: {}", operation.error());
        } else {
            LOG_INFO("Response: {} capabilities", operation.value().size());
        }

        return operation;
    }
} // namespace service::api