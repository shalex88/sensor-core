#include "ApiControllerFactory.h"

#include "api/ApiController.h"
#include "api/GrpcTransport.h"
#include "api/RequestHandler.h"
#include "common/config/ConfigManager.h"
#include "common/network/NetworkUtils.h"
#include "core/ICore.h"

namespace service::api {
    std::unique_ptr<ApiController> ApiControllerFactory::createController(
        std::unique_ptr<core::ICore> core, const common::ApiConfig& config) {
        if (!core) {
            throw std::invalid_argument("Core cannot be null");
        }

        auto server_address = config.server_address;
        if (server_address == "0.0.0.0:50051") {
            const auto ip_result = common::network::getPrimaryIpAddress();
            if (ip_result.isError()) {
                throw std::runtime_error("Failed to get device IP: " + ip_result.error());
            }
            server_address = ip_result.value() + ":50051";
        }

        if (config.api == "grpc") {
            auto request_handler = std::make_unique<RequestHandler>(std::move(core));
            auto transport = std::make_unique<GrpcTransport>(*request_handler);
            return std::make_unique<ApiController>(std::move(request_handler), std::move(transport), server_address);
        }

        throw std::invalid_argument("Unknown API controller type: " + config.api);
    }
} // namespace service::api