#include "CoreFactory.h"

#include "common/config/ConfigManager.h"
#include "core/Core.h"
#include "infrastructure/camera/hal/ICamera.h"

namespace service::core {
    std::unique_ptr<ICore> CoreFactory::createCore(std::unique_ptr<infrastructure::ICamera> camera,
                                                   const common::CoreConfig& config) {
        if (!camera) {
            throw std::invalid_argument("Camera cannot be null");
        }

        if (config.camera == "core") {
            return std::make_unique<Core>(std::move(camera));
        }

        throw std::invalid_argument("Unknown core type");
    }
} // namespace service::core
