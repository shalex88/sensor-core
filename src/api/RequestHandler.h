#pragma once

#include <atomic>
#include <memory>

#include "api/IRequestHandler.h"
#include "common/types/CameraTypes.h"
#include "common/types/Result.h"

namespace service::core {
    class ICore;
}

namespace service::api {
    class RequestHandler final : public IRequestHandler {
    public:
        explicit RequestHandler(std::unique_ptr<core::ICore> core);
        ~RequestHandler() override;

        Result<void> start() override;
        Result<void> stop() override;
        bool isRunning() const override;

        // Capability-aware request methods
        Result<void> setZoom(uint32_t camera_id, common::types::zoom zoom_level) const override;
        Result<common::types::zoom> getZoom(uint32_t camera_id) const override;
        Result<void> goToMinZoom(uint32_t camera_id) const override;
        Result<void> goToMaxZoom(uint32_t camera_id) const override;

        Result<void> setFocus(uint32_t camera_id, common::types::focus focus_value) const override;
        Result<common::types::focus> getFocus(uint32_t camera_id) const override;
        Result<void> enableAutoFocus(uint32_t camera_id, bool on) const override;

        Result<common::types::info> getInfo(uint32_t camera_id) const override;

        Result<void> stabilize(uint32_t camera_id, bool on) const override;

        Result<common::capabilities::CapabilityList> getCapabilities(uint32_t camera_id) const override;

    private:
        std::unique_ptr<core::ICore> core_;
        std::atomic<bool> running_;
    };
}
