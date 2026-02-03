#pragma once

#include <memory>
#include <grpcpp/client_context.h>
#include <grpcpp/channel.h>

#include "api/proto/video_service.grpc.pb.h"
#include "infrastructure/clients/IVideoServiceClient.h"

namespace service::infrastructure {
    class VideoServiceClient : public IVideoServiceClient {
    public:
        explicit VideoServiceClient(std::shared_ptr<grpc::ChannelInterface> channel);
        ~VideoServiceClient() override = default;

        // Video operations
        Result<void> enableOptionalElement(const std::string& element) override;
        Result<void> disableOptionalElement(const std::string& element) override;

    private:
        std::unique_ptr<video::VideoService::Stub> stub_;

        /**
         * Helper to handle gRPC call results
         * Converts gRPC status to Result<T> errors
         */
        Result<void> handleGrpcVoidError(const grpc::Status& status, const std::string& method) {
            if (!status.ok()) {
                return Result<void>::error(std::string("video_service.") + method + ": " + status.error_message());
            }
            return Result<void>::success();
        }
    };
} // namespace service::infrastructure
