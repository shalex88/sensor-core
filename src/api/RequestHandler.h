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
        Result<void> setZoom(common::types::zoom zoom_level) const override;
        Result<common::types::zoom> getZoom() const override;
        Result<void> goToMinZoom() const override;
        Result<void> goToMaxZoom() const override;

        Result<void> setFocus(common::types::focus focus_value) const override;
        Result<common::types::focus> getFocus() const override;
        Result<void> enableAutoFocus(bool on) const override;

        Result<common::types::info> getInfo() const override;

        Result<void> stabilize(bool on) const override;

        Result<common::capabilities::CapabilityList> getCapabilities() const override;

    private:
        std::unique_ptr<core::ICore> core_;
        std::atomic<bool> running_;
    };
}
