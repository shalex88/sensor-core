#pragma once

#include <memory>

namespace service::common {
    struct CoreConfig;
} // namespace service::common

namespace service::infrastructure {
    class ICamera;
} // namespace service::infrastructure

namespace service::core {
    class ICore;

    class CoreFactory {
    public:
        static std::unique_ptr<ICore> createCore(
            std::unique_ptr<infrastructure::ICamera> camera, const common::CoreConfig& config);
    };
} // namespace service::core
