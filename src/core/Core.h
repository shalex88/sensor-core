#pragma once

#include <memory>

#include "common/types/CameraTypes.h"
#include "common/types/Result.h"
#include "common/config/ConfigManager.h"
#include "core/ICore.h"

namespace service::infrastructure {
    class GrpcClientManager;
    class ICameraServiceClient;
} // namespace service::infrastructure

namespace service::core {
    class Core final : public ICore {
    public:
        explicit Core(const common::InfrastructureConfig& infrastructure_config);
        ~Core() override;

        // ICore implementation
        Result<void> start() override;
        Result<void> stop() override;

        // Business methods for zoom operations
        Result<void> setZoom(uint32_t camera_id, common::types::zoom zoom_level) const override;
        Result<common::types::zoom> getZoom(uint32_t camera_id) const override;
        Result<void> goToMinZoom(uint32_t camera_id) const override;
        Result<void> goToMaxZoom(uint32_t camera_id) const override;

        // Business methods for focus operations
        Result<void> setFocus(uint32_t camera_id, common::types::focus focus_value) const override;
        Result<common::types::focus> getFocus(uint32_t camera_id) const override;
        Result<void> enableAutoFocus(uint32_t camera_id, bool on) const override;

        // Business methods for info operations
        Result<common::types::info> getInfo(uint32_t camera_id) const override;

        // Business methods for advanced operations
        Result<void> stabilize(uint32_t camera_id, bool on) const override;

        // Capability inquiry
        Result<common::capabilities::CapabilityList> getCapabilities(uint32_t camera_id) const override;

    private:
        bool isRunning() const;
        common::InfrastructureConfig infrastructure_config_;
        std::unique_ptr<infrastructure::GrpcClientManager> client_manager_;
        bool is_running_;
    };
} // namespace service::core
