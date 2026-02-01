#pragma once

#include <memory>

#include "common/types/CameraTypes.h"
#include "common/types/Result.h"
#include "core/ICore.h"

namespace service::infrastructure {
    class ICamera;
}

namespace service::core {
    class Core final : public ICore {
    public:
        explicit Core(std::unique_ptr<infrastructure::ICamera> camera);
        ~Core() override;

        // ICore implementation
        Result<void> start() override;
        Result<void> stop() override;

        // Business methods for zoom operations
        Result<void> setZoom(common::types::zoom zoom_level) const override;
        Result<common::types::zoom> getZoom() const override;
        Result<void> goToMinZoom() const override;
        Result<void> goToMaxZoom() const override;

        // Business methods for focus operations
        Result<void> setFocus(common::types::focus focus_value) const override;
        Result<common::types::focus> getFocus() const override;
        Result<void> enableAutoFocus(bool on) const override;

        // Business methods for info operations
        Result<common::types::info> getInfo() const override;

        // Business methods for advanced operations
        Result<void> stabilize(bool on) const override;

        // Capability inquiry
        Result<common::capabilities::CapabilityList> getCapabilities() const override;

    private:
        bool isRunning() const;
        std::unique_ptr<infrastructure::ICamera> camera_;
        bool is_running_;
    };
} // namespace service::core
