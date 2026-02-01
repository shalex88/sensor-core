#include "CameraFactory.h"

#include "common/config/ConfigManager.h"
#include "common/logger/Logger.h"
#include "infrastructure/camera/devices/AdimecCamera.h"
#include "infrastructure/camera/devices/FakeAdvancedCamera.h"
#include "infrastructure/camera/devices/FakeSimpleCamera.h"
#include "infrastructure/camera/devices/MwirCamera.h"
#include "infrastructure/camera/devices/SonyCamera.h"
#include "infrastructure/camera/hal/Camera.h"
#include "infrastructure/camera/hal/ICamera.h"
#include "infrastructure/camera/protocol/genicam/FpgaTransport.h"
#include "infrastructure/camera/protocol/genicam/GenicamProtocol.h"
#include "infrastructure/camera/protocol/itl/ItlProtocol.h"
#include "infrastructure/camera/protocol/visca/ViscaProtocol.h"
#include "infrastructure/camera/transport/ethernet/TcpClient.h"
#include "infrastructure/camera/transport/uart/Uart.h"
#include "infrastructure/fpga/VideoChannel.h"

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
        LOG_DEBUG("Creating camera: {}", config.camera);
        for (size_t i = 0; i < config.endpoints.size(); ++i) {
            const auto& [address, configuration] = config.endpoints[i];
            LOG_DEBUG("Endpoint[{}]: address={}, config: {}", i, address, formatConfiguration(configuration));
        }

        if (config.video_channel != std::nullopt) {
            VideoChannel(config.video_channel.value());
        }

        if (config.camera == "adimec") {
            if (config.endpoints.size() < 1 || config.endpoints.size() > 2) {
                throw std::invalid_argument(
                    "Adimec requires 1 or 2 endpoints");
            }

            const auto& [camera_address, camera_configuration] = config.endpoints[0];
            auto camera_transport = std::make_unique<FpgaTransport>(camera_address);
            auto camera_protocol = std::make_unique<GenicamProtocol>(std::move(camera_transport));

            const auto& [lens_address, lens_configuration] = config.endpoints[1];
            auto lens_transport = std::make_unique<TcpClient>(lens_address);
            auto lens_protocol = std::make_unique<ItlProtocol>(std::move(lens_transport));

            auto camera = std::make_unique<AdimecCamera>(std::move(camera_protocol), std::move(lens_protocol));
            return std::make_unique<Camera>(std::move(camera));
        }

        if (config.camera == "sony") {
            if (config.endpoints.size() != 1) {
                throw std::invalid_argument("Sony requires at 1 endpoint");
            }
            const auto& [camera_address, camera_configuration] = config.endpoints[0];
            auto transport = std::make_unique<Uart>(camera_address, camera_configuration.at("baud_rate"));
            auto protocol = std::make_unique<ViscaProtocol>(std::move(transport));

            auto camera = std::make_unique<SonyCamera>(std::move(protocol));
            return std::make_unique<Camera>(std::move(camera));
        }

        if (config.camera == "mwir") {
            if (config.endpoints.size() != 1) {
                throw std::invalid_argument("MWIR requires at 1 endpoint");
            }

            const auto& [camera_address, camera_configuration] = config.endpoints[0];
            auto transport = std::make_unique<TcpClient>(camera_address);
            auto protocol = std::make_unique<ItlProtocol>(std::move(transport));

            auto camera = std::make_unique<MwirCamera>(std::move(protocol));
            return std::make_unique<Camera>(std::move(camera));
        }

        if (config.camera == "fake_advanced") {
            if (!config.endpoints.empty()) {
                throw std::invalid_argument("Fake camera should not have endpoints");
            }

            auto camera = std::make_unique<FakeAdvancedCamera>();
            return std::make_unique<Camera>(std::move(camera));
        }

        if (config.camera == "fake_simple") {
            if (!config.endpoints.empty()) {
                throw std::invalid_argument("Fake camera should not have endpoints");
            }

            auto camera = std::make_unique<FakeSimpleCamera>();
            return std::make_unique<Camera>(std::move(camera));
        }

        throw std::invalid_argument("Unknown camera type: " + config.camera);
    }
} // namespace service::infrastructure