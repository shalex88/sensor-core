#pragma once

#include <memory>

namespace service::common {
    struct InfrastructureConfig;
} // namespace service::common

namespace service::core {
    class ICore;

    class CoreFactory {
    public:
        static std::unique_ptr<ICore> createCore(const common::InfrastructureConfig& config);
    };
} // namespace service::core
