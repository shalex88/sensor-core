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

        auto server = config.server;
        if (server == "0.0.0.0") {
            const auto ip_result = common::network::getPrimaryIpAddress();
            if (ip_result.isError()) {
                throw std::runtime_error("Failed to get device IP: " + ip_result.error());
            }
            server = ip_result.value();
        }

        auto request_handler = std::make_unique<RequestHandler>(std::move(core));

        if (config.api == "grpc") {
            auto transport = std::make_unique<GrpcTransport>(*request_handler);
            return std::make_unique<ApiController>(std::move(request_handler), std::move(transport), server, config.port);
        }

        if (config.api == "rest") {
            auto transport = std::make_unique<RestTransport>(*request_handler);
            return std::make_unique<ApiController>(std::move(request_handler), std::move(transport), server, config.port);
        }

        throw std::invalid_argument("Unknown API controller type: " + config.api);
    }
} // namespace service::api