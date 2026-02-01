#pragma once

#include <memory>

namespace service::common {
    struct InfrastructureConfig;
} // namespace service::common

namespace service::infrastructure {
    class ICamera;

    class CameraFactory {
    public:
        static std::unique_ptr<ICamera> createCamera(const common::InfrastructureConfig& config);
    };
} // namespace service::infrastructure
