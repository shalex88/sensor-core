#pragma once

#include "api/ITransport.h"
#include "api/proto/camera_service.grpc.pb.h" //TODO: can move to implementation file?
#include "common/types/Result.h"

namespace service::api {
    class IRequestHandler;
    class GrpcCallbackHandler;

    class GrpcTransport final : public ITransport {
    public:
        explicit GrpcTransport(IRequestHandler& request_handler);
        ~GrpcTransport() override;

        Result<void> start(const std::string& server, uint16_t port) override;
        Result<void> stop() override;
        Result<void> runLoop() override;

    private:
        std::unique_ptr<GrpcCallbackHandler> callback_handler_;
        std::unique_ptr<grpc::Server> server_;
        bool is_running_{false};
    };
} // namespace service::api
