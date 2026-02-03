#include "infrastructure/clients/VideoServiceClient.h"

#include "common/logger/Logger.h"

namespace service::infrastructure {
    VideoServiceClient::VideoServiceClient(std::shared_ptr<grpc::ChannelInterface> channel)
        : stub_(video::VideoService::NewStub(channel)) {
        if (!stub_) {
            throw std::runtime_error("Failed to create video_service stub");
        }
    }

    // Video operations
    Result<void> VideoServiceClient::enableOptionalElement(const std::string& element) {
        video::EnableOptionalElementRequest request;
        request.set_element(element);

        video::EnableOptionalElementResponse response;
        grpc::ClientContext context;

        const auto status = stub_->EnableOptionalElement(&context, request, &response);
        return handleGrpcVoidError(status, "EnableOptionalElement");
    }

    Result<void> VideoServiceClient::disableOptionalElement(const std::string& element) {
        video::DisableOptionalElementRequest request;
        request.set_element(element);

        video::DisableOptionalElementResponse response;
        grpc::ClientContext context;

        const auto status = stub_->DisableOptionalElement(&context, request, &response);
        return handleGrpcVoidError(status, "DisableOptionalElement");
    }
} // namespace service::infrastructure
