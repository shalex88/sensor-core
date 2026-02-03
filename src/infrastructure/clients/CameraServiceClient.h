#pragma once

#include <memory>
#include <grpcpp/client_context.h>
#include <grpcpp/channel.h>

#include "api/proto/camera_service.grpc.pb.h"
#include "common/types/CameraCapabilities.h"
#include "infrastructure/clients/ICameraServiceClient.h"

namespace service::infrastructure {
    class CameraServiceClient : public ICameraServiceClient {
    public:
        explicit CameraServiceClient(std::shared_ptr<grpc::ChannelInterface> channel);
        ~CameraServiceClient() override = default;

        // Zoom operations
        Result<void> setZoom(common::types::zoom zoom_level) override;
        Result<common::types::zoom> getZoom() override;
        Result<void> goToMinZoom() override;
        Result<void> goToMaxZoom() override;

        // Focus operations
        Result<void> setFocus(common::types::focus focus_value) override;
        Result<common::types::focus> getFocus() override;
        Result<void> enableAutoFocus(bool on) override;

        // Device info
        Result<common::types::info> getInfo() override;

        // Advanced operations
        Result<void> stabilize(bool on) override;

        // Capabilities
        Result<common::capabilities::CapabilityList> getCapabilities() override;

    private:
        std::unique_ptr<camera::v1::CameraService::Stub> stub_;

        /**
         * Helper to handle gRPC call results
         * Converts gRPC status to Result<T> errors
         */
        template<typename T>
        Result<T> handleGrpcError(const grpc::Status& status, const std::string& method) {
            if (!status.ok()) {
                return Result<T>::error(std::string("camera_service.") + method + ": " + status.error_message());
            }
            return Result<T>::success();
        }

        Result<void> handleGrpcVoidError(const grpc::Status& status, const std::string& method) {
            if (!status.ok()) {
                return Result<void>::error(std::string("camera_service.") + method + ": " + status.error_message());
            }
            return Result<void>::success();
        }
    };
} // namespace service::infrastructure
