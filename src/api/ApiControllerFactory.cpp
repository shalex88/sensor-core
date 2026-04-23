#include "ApiControllerFactory.h"

#include "api/ApiController.h"
#include "api/RequestHandler.h"
#include "api/grpc/GrpcTransport.h"
#include "api/rest/RestTransport.h"
#include "common/config/ConfigManager.h"
#include "common/network/NetworkUtils.h"
#include "core/ICore.h"

namespace service::api {
    std::unique_ptr<ApiController> ApiControllerFactory::createController(
        std::unique_ptr<core::ICore> core, const common::ApiConfig& config) {
        if (!core) {
            throw std::invalid_argument("Core cannot be null");
        }

        const auto server_ip = common::network::getPrimaryIpAddress();
        if (server_ip.isError()) {
            throw std::runtime_error("Failed to get device IP: " + server_ip.error());
        }

        auto request_handler = std::make_unique<RequestHandler>(std::move(core));

        if (config.api == "grpc") {
            auto transport = std::make_unique<GrpcTransport>(*request_handler);
            return std::make_unique<ApiController>(std::move(request_handler), std::move(transport), server_ip.value(), config.port);
        }

        if (config.api == "rest") {
            auto transport = std::make_unique<RestTransport>(*request_handler);
            return std::make_unique<ApiController>(std::move(request_handler), std::move(transport), server_ip.value(), config.port);
        }

        throw std::invalid_argument("Unknown API controller type: " + config.api);
    }
} // namespace service::api