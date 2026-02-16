#include "infrastructure/clients/VideoServiceClient.h"

#include <google/protobuf/empty.pb.h>

#include "common/logger/Logger.h"

namespace service::infrastructure {
    VideoServiceClient::VideoServiceClient(std::shared_ptr<grpc::ChannelInterface> channel)
        : stub_(video::v1::VideoService::NewStub(channel)) {
        if (!stub_) {
            throw std::runtime_error("Failed to create video_service stub");
        }
    }

    // Video operations
    Result<void> VideoServiceClient::SetVideoCapabilityState(const std::string& capability, const bool enable) {
        video::v1::SetVideoCapabilityStateRequest request;
        request.set_capability(capability);
        request.set_enable(enable);

        google::protobuf::Empty response;
        grpc::ClientContext context;

        const auto status = stub_->SetVideoCapabilityState(&context, request, &response);
        return handleGrpcVoidError(status, "SetVideoCapabilityState");
    }

    Result<bool> VideoServiceClient::getVideoCapabilityState(const std::string& capability) {
        video::v1::GetVideoCapabilityStateRequest request;
        request.set_capability(capability);
        video::v1::GetVideoCapabilityStateResponse response;
        grpc::ClientContext context;

        const auto status = stub_->GetVideoCapabilityState(&context, request, &response);
        if (!status.ok()) {
            return Result<bool>::error(
                std::string("video_service.GetVideoCapabilityState: ") + status.error_message()
            );
        }

        return Result<bool>::success(response.enable());
    }

    Result<std::vector<std::string>> VideoServiceClient::getVideoCapabilities() {
        google::protobuf::Empty request;
        video::v1::GetVideoCapabilitiesResponse response;
        grpc::ClientContext context;

        const auto status = stub_->GetVideoCapabilities(&context, request, &response);
        if (!status.ok()) {
            return Result<std::vector<std::string>>::error(
                std::string("video_service.GetVideoCapabilities: ") + status.error_message()
            );
        }

        std::vector<std::string> capabilities;
        capabilities.reserve(response.capabilities_size());
        for (const auto& capability : response.capabilities()) {
            capabilities.push_back(capability);
        }

        return Result<std::vector<std::string>>::success(capabilities);
    }
} // namespace service::infrastructure
