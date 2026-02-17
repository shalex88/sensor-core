#pragma once

#include "api/proto/core_service.grpc.pb.h"

namespace service::api {
    class IRequestHandler;

    class GrpcCallbackHandler final : public core::v1::CoreService::CallbackService {
    public:
        explicit GrpcCallbackHandler(IRequestHandler& request_handler);

        grpc::ServerUnaryReactor* SetZoom(
            grpc::CallbackServerContext* context,
            const core::v1::SetZoomRequest* request,
            core::v1::SetZoomResponse* response) override;

        grpc::ServerUnaryReactor* SetFocus(
            grpc::CallbackServerContext* context,
            const core::v1::SetFocusRequest* request,
            core::v1::SetFocusResponse* response) override;

        grpc::ServerUnaryReactor* GetZoom(
            grpc::CallbackServerContext* context,
            const core::v1::GetZoomRequest* request,
            core::v1::GetZoomResponse* response) override;

        grpc::ServerUnaryReactor* GetFocus(
            grpc::CallbackServerContext* context,
            const core::v1::GetFocusRequest* request,
            core::v1::GetFocusResponse* response) override;

        grpc::ServerUnaryReactor* GetInfo(
            grpc::CallbackServerContext* context,
            const core::v1::GetInfoRequest* request,
            core::v1::GetInfoResponse* response) override;

        grpc::ServerUnaryReactor* GetCapabilities(
            grpc::CallbackServerContext* context,
            const core::v1::GetCapabilitiesRequest* request,
            core::v1::GetCapabilitiesResponse* response) override;

        grpc::ServerUnaryReactor* GoToMinZoom(
            grpc::CallbackServerContext* context,
            const core::v1::GoToMinZoomRequest* request,
            core::v1::GoToMinZoomResponse* response) override;

        grpc::ServerUnaryReactor* GoToMaxZoom(
            grpc::CallbackServerContext* context,
            const core::v1::GoToMaxZoomRequest* request,
            core::v1::GoToMaxZoomResponse* response) override;

        grpc::ServerUnaryReactor* SetAutoFocus(
            grpc::CallbackServerContext* context,
            const core::v1::SetAutoFocusRequest* request,
            google::protobuf::Empty* response) override;

        grpc::ServerUnaryReactor* GetAutoFocus(
            grpc::CallbackServerContext* context,
            const core::v1::GetAutoFocusRequest* request,
            core::v1::GetAutoFocusResponse* response) override;

        grpc::ServerUnaryReactor* SetStabilization(
            grpc::CallbackServerContext* context,
            const core::v1::SetStabilizationRequest* request,
            google::protobuf::Empty* response) override;

        grpc::ServerUnaryReactor* GetStabilization(
            grpc::CallbackServerContext* context,
            const core::v1::GetStabilizationRequest* request,
            core::v1::GetStabilizationResponse* response) override;

        // Video operations
        grpc::ServerUnaryReactor* SetVideoCapabilityState(
            grpc::CallbackServerContext* context,
            const core::v1::SetVideoCapabilityStateRequest* request,
            google::protobuf::Empty* response) override;

        grpc::ServerUnaryReactor* GetVideoCapabilities(
            grpc::CallbackServerContext* context,
            const core::v1::GetVideoCapabilitiesRequest* request,
            core::v1::GetVideoCapabilitiesResponse* response) override;

        grpc::ServerUnaryReactor* GetVideoCapabilityState(
            grpc::CallbackServerContext* context,
            const core::v1::GetVideoCapabilityStateRequest* request,
            core::v1::GetVideoCapabilityStateResponse* response) override;

    private:
        IRequestHandler& request_handler_;
    };
} // namespace service::api
