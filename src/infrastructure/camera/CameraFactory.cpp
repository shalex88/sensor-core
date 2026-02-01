#include "CameraFactory.h"

#include "common/config/ConfigManager.h"
#include "common/logger/Logger.h"
#include "infrastructure/camera/hal/Camera.h"
#include "infrastructure/camera/hal/ICamera.h"

namespace service::infrastructure {
    namespace {
        std::string formatConfiguration(const std::unordered_map<std::string, std::string>& configuration) {
            std::string result;
            for (const auto& [key, value] : configuration) {
                if (!result.empty()) {
                    result.append(", ");
                }
                result.append(key).append("=").append(value);
            }
            return result;
        }
    } // unnamed namespace

    std::unique_ptr<ICamera> CameraFactory::createCamera(const common::InfrastructureConfig& config) {
        throw std::invalid_argument("Unknown camera type: " + config.camera);
    }
} // namespace service::infrastructure