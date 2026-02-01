#pragma once

#include "api/proto/camera_service.grpc.pb.h"

namespace service::api {
    class IRequestHandler;

    class GrpcCallbackHandler final : public camera::v1::CameraService::CallbackService {
    public:
        explicit GrpcCallbackHandler(IRequestHandler& request_handler);

        grpc::ServerUnaryReactor* SetZoom(
            grpc::CallbackServerContext* context,
            const camera::v1::SetZoomRequest* request,
            camera::v1::SetZoomResponse* response) override;

        grpc::ServerUnaryReactor* SetFocus(
            grpc::CallbackServerContext* context,
            const camera::v1::SetFocusRequest* request,
            camera::v1::SetFocusResponse* response) override;

        grpc::ServerUnaryReactor* GetZoom(
            grpc::CallbackServerContext* context,
            const google::protobuf::Empty* request,
            camera::v1::GetZoomResponse* response) override;

        grpc::ServerUnaryReactor* GetFocus(
            grpc::CallbackServerContext* context,
            const google::protobuf::Empty* request,
            camera::v1::GetFocusResponse* response) override;

        grpc::ServerUnaryReactor* GetInfo(
            grpc::CallbackServerContext* context,
            const google::protobuf::Empty* request,
            camera::v1::GetInfoResponse* response) override;

        grpc::ServerUnaryReactor* GetCapabilities(
            grpc::CallbackServerContext* context,
            const google::protobuf::Empty* request,
            camera::v1::GetCapabilitiesResponse* response) override;

        grpc::ServerUnaryReactor* GoToMinZoom(
            grpc::CallbackServerContext* context,
            const google::protobuf::Empty* request,
            camera::v1::GoToMinZoomResponse* response) override;

        grpc::ServerUnaryReactor* GoToMaxZoom(
            grpc::CallbackServerContext* context,
            const google::protobuf::Empty* request,
            camera::v1::GoToMaxZoomResponse* response) override;

        grpc::ServerUnaryReactor* SetAutoFocus(
            grpc::CallbackServerContext* context,
            const camera::v1::SetAutoFocusRequest* request,
            google::protobuf::Empty* response) override;

        grpc::ServerUnaryReactor* SetStabilization(
            grpc::CallbackServerContext* context,
            const camera::v1::SetStabilizationRequest* request,
            google::protobuf::Empty* response) override;

    private:
        IRequestHandler& request_handler_;
    };
} // namespace service::api