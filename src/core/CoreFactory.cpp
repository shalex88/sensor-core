#include "CoreFactory.h"

#include "common/config/ConfigManager.h"
#include "core/Core.h"

namespace service::core {
    std::unique_ptr<ICore> CoreFactory::createCore(const common::InfrastructureConfig& config) {
        return std::make_unique<Core>(config);
    }
} // namespace service::core
