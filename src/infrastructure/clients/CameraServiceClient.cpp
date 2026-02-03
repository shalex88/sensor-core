#include "infrastructure/clients/CameraServiceClient.h"

#include <google/protobuf/empty.pb.h>
#include "common/logger/Logger.h"

namespace service::infrastructure {
    CameraServiceClient::CameraServiceClient(std::shared_ptr<grpc::ChannelInterface> channel)
        : stub_(camera::v1::CameraService::NewStub(channel)) {
        if (!stub_) {
            throw std::runtime_error("Failed to create camera_service stub");
        }
    }

    // Zoom operations
    Result<void> CameraServiceClient::setZoom(common::types::zoom zoom_level) {
        camera::v1::SetZoomRequest request;
        request.set_zoom(zoom_level);

        camera::v1::SetZoomResponse response;
        grpc::ClientContext context;

        const auto status = stub_->SetZoom(&context, request, &response);
        return handleGrpcVoidError(status, "SetZoom");
    }

    Result<common::types::zoom> CameraServiceClient::getZoom() {
        google::protobuf::Empty request;
        camera::v1::GetZoomResponse response;
        grpc::ClientContext context;

        const auto status = stub_->GetZoom(&context, request, &response);
        if (!status.ok()) {
            return Result<common::types::zoom>::error(
                std::string("camera_service.GetZoom: ") + status.error_message()
            );
        }
        return Result<common::types::zoom>::success(static_cast<common::types::zoom>(response.zoom()));
    }

    Result<void> CameraServiceClient::goToMinZoom() {
        google::protobuf::Empty request;
        camera::v1::GoToMinZoomResponse response;
        grpc::ClientContext context;

        const auto status = stub_->GoToMinZoom(&context, request, &response);
        return handleGrpcVoidError(status, "GoToMinZoom");
    }

    Result<void> CameraServiceClient::goToMaxZoom() {
        google::protobuf::Empty request;
        camera::v1::GoToMaxZoomResponse response;
        grpc::ClientContext context;

        const auto status = stub_->GoToMaxZoom(&context, request, &response);
        return handleGrpcVoidError(status, "GoToMaxZoom");
    }

    // Focus operations
    Result<void> CameraServiceClient::setFocus(common::types::focus focus_value) {
        camera::v1::SetFocusRequest request;
        request.set_focus(focus_value);

        camera::v1::SetFocusResponse response;
        grpc::ClientContext context;

        const auto status = stub_->SetFocus(&context, request, &response);
        return handleGrpcVoidError(status, "SetFocus");
    }

    Result<common::types::focus> CameraServiceClient::getFocus() {
        google::protobuf::Empty request;
        camera::v1::GetFocusResponse response;
        grpc::ClientContext context;

        const auto status = stub_->GetFocus(&context, request, &response);
        if (!status.ok()) {
            return Result<common::types::focus>::error(
                std::string("camera_service.GetFocus: ") + status.error_message()
            );
        }
        return Result<common::types::focus>::success(static_cast<common::types::focus>(response.focus()));
    }

    Result<void> CameraServiceClient::enableAutoFocus(bool on) {
        camera::v1::SetAutoFocusRequest request;
        request.set_enable(on);

        google::protobuf::Empty response;
        grpc::ClientContext context;

        const auto status = stub_->SetAutoFocus(&context, request, &response);
        return handleGrpcVoidError(status, "SetAutoFocus");
    }

    // Device info
    Result<common::types::info> CameraServiceClient::getInfo() {
        google::protobuf::Empty request;
        camera::v1::GetInfoResponse response;
        grpc::ClientContext context;

        const auto status = stub_->GetInfo(&context, request, &response);
        if (!status.ok()) {
            return Result<common::types::info>::error(
                std::string("camera_service.GetInfo: ") + status.error_message()
            );
        }
        return Result<common::types::info>::success(response.info());
    }

    // Advanced operations
    Result<void> CameraServiceClient::stabilize(bool on) {
        camera::v1::SetStabilizationRequest request;
        request.set_enable(on);

        google::protobuf::Empty response;
        grpc::ClientContext context;

        const auto status = stub_->SetStabilization(&context, request, &response);
        return handleGrpcVoidError(status, "SetStabilization");
    }

    // Capabilities
    Result<common::capabilities::CapabilityList> CameraServiceClient::getCapabilities() {
        google::protobuf::Empty request;
        camera::v1::GetCapabilitiesResponse response;
        grpc::ClientContext context;

        const auto status = stub_->GetCapabilities(&context, request, &response);
        if (!status.ok()) {
            return Result<common::capabilities::CapabilityList>::error(
                std::string("camera_service.GetCapabilities: ") + status.error_message()
            );
        }

        // Convert protobuf capabilities to domain capabilities
        common::capabilities::CapabilityList capabilities;
        for (const auto& proto_cap : response.capabilities()) {
            switch (proto_cap) {
                case camera::v1::CAPABILITY_ZOOM:
                    capabilities.push_back(common::capabilities::Capability::Zoom);
                    break;
                case camera::v1::CAPABILITY_FOCUS:
                    capabilities.push_back(common::capabilities::Capability::Focus);
                    break;
                case camera::v1::CAPABILITY_AUTO_FOCUS:
                    capabilities.push_back(common::capabilities::Capability::AutoFocus);
                    break;
                case camera::v1::CAPABILITY_INFO:
                    capabilities.push_back(common::capabilities::Capability::Info);
                    break;
                case camera::v1::CAPABILITY_STABILIZATION:
                    capabilities.push_back(common::capabilities::Capability::Stabilization);
                    break;
                default:
                    LOG_WARN("Unknown camera capability: {}", proto_cap);
                    break;
            }
        }
        return Result<common::capabilities::CapabilityList>::success(capabilities);
    }
} // namespace service::infrastructure
