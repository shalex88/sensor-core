#include "GrpcCallbackHandler.h"

#include <future>
#include <grpcpp/grpcpp.h>

#include "api/IRequestHandler.h"
#include "common/logger/Logger.h"
#include "common/types/CameraCapabilities.h"

namespace service::api {
    namespace {
        template<typename RequestType, typename ResponseType, typename ProcessFunc>
        grpc::ServerUnaryReactor* handleGrpcSyncRequest(
            grpc::CallbackServerContext* context,
            const RequestType* request,
            ResponseType* response,
            ProcessFunc process_function) {
            auto* const reactor = context->DefaultReactor();
            const auto status = [&]() {
                if (auto result = process_function(request, response); result.isError()) {
                    return grpc::Status(grpc::StatusCode::INTERNAL, result.error());
                }
                return grpc::Status::OK;
            }();

            reactor->Finish(status);
            return reactor;
        }

        template<typename RequestType, typename ResponseType, typename ProcessFunc>
        grpc::ServerUnaryReactor* handleGrpcAsyncRequest(
            grpc::CallbackServerContext* context,
            const RequestType* request,
            ResponseType* response,
            ProcessFunc process_function) {
            auto* const reactor = context->DefaultReactor();
            const auto deadline = grpc::Timespec2Timepoint(context->raw_deadline());

            const auto now = std::chrono::system_clock::now();
            const auto remaining_time = deadline > now ? deadline - now : std::chrono::seconds(0);

            std::future<grpc::Status> future = std::async(std::launch::async, [request, response, process_function] {
                if (auto result = process_function(request, response); result.isError()) {
                    return grpc::Status(grpc::StatusCode::INTERNAL, result.error());
                }
                return grpc::Status::OK;
            });

            //TODO: handle task execution abort if deadline exceeds, think of a way to cancel and undo the task

            if (future.wait_for(remaining_time) == std::future_status::timeout) {
                LOG_ERROR("Request exceeded deadline during processing.");
                reactor->Finish(grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED, "Processing exceeded deadline"));
            } else {
                reactor->Finish(future.get());
            }

            return reactor;
        }

        core::v1::Capability toProto(const common::capabilities::Capability capability) {
            switch (capability) {
            case common::capabilities::Capability::Zoom:
                return core::v1::CAPABILITY_ZOOM;
            case common::capabilities::Capability::Focus:
                return core::v1::CAPABILITY_FOCUS;
            case common::capabilities::Capability::AutoFocus:
                return core::v1::CAPABILITY_AUTO_FOCUS;
            case common::capabilities::Capability::Info:
                return core::v1::CAPABILITY_INFO;
            case common::capabilities::Capability::Stabilization:
                return core::v1::CAPABILITY_STABILIZATION;
            default:
                return core::v1::CAPABILITY_UNSPECIFIED;
            }
        }
    } // unnamed namespace

    GrpcCallbackHandler::GrpcCallbackHandler(IRequestHandler& request_handler)
        : request_handler_(request_handler) {
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::SetZoom(
        grpc::CallbackServerContext* context,
        const core::v1::SetZoomRequest* request,
        core::v1::SetZoomResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::SetZoomRequest* req, core::v1::SetZoomResponse*) {
                return request_handler_.setZoom(req->camera_id(), req->zoom());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::SetFocus(
        grpc::CallbackServerContext* context,
        const core::v1::SetFocusRequest* request,
        core::v1::SetFocusResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::SetFocusRequest* req, core::v1::SetFocusResponse*) {
                return request_handler_.setFocus(req->camera_id(), req->focus());
            });

    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GetZoom(
        grpc::CallbackServerContext* context,
        const core::v1::GetZoomRequest* request,
        core::v1::GetZoomResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GetZoomRequest* req, core::v1::GetZoomResponse* resp) {
                const auto result = request_handler_.getZoom(req->camera_id());
                if (result.isSuccess()) {
                    resp->set_zoom(result.value());
                    return Result<void>::success();
                }
                return Result<void>::error(result.error());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GetFocus(
        grpc::CallbackServerContext* context,
        const core::v1::GetFocusRequest* request,
        core::v1::GetFocusResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GetFocusRequest* req, core::v1::GetFocusResponse* resp) {
                const auto result = request_handler_.getFocus(req->camera_id());
                if (result.isSuccess()) {
                    resp->set_focus(result.value());
                    return Result<void>::success();
                }
                return Result<void>::error(result.error());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GetInfo(
        grpc::CallbackServerContext* context,
        const core::v1::GetInfoRequest* request,
        core::v1::GetInfoResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GetInfoRequest* req, core::v1::GetInfoResponse* resp) {
                const auto result = request_handler_.getInfo(req->camera_id());
                if (result.isSuccess()) {
                    resp->set_info(result.value());
                    return Result<void>::success();
                }
                return Result<void>::error(result.error());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GetCapabilities(
        grpc::CallbackServerContext* context,
        const core::v1::GetCapabilitiesRequest* request,
        core::v1::GetCapabilitiesResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GetCapabilitiesRequest* req, core::v1::GetCapabilitiesResponse* resp) {
                const auto result = request_handler_.getCapabilities(req->camera_id());
                if (result.isError()) {
                    return Result<void>::error(result.error());
                }

                for (const auto capability : result.value()) {
                    resp->add_capabilities(toProto(capability));
                }

                return Result<void>::success();
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GoToMinZoom(
        grpc::CallbackServerContext* context,
        const core::v1::GoToMinZoomRequest* request,
        core::v1::GoToMinZoomResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GoToMinZoomRequest* req, core::v1::GoToMinZoomResponse*) {
                const auto result = request_handler_.goToMinZoom(req->camera_id());
                if (result.isSuccess()) {
                    return Result<void>::success();
                }
                return Result<void>::error(result.error());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GoToMaxZoom(
        grpc::CallbackServerContext* context,
        const core::v1::GoToMaxZoomRequest* request,
        core::v1::GoToMaxZoomResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GoToMaxZoomRequest* req, core::v1::GoToMaxZoomResponse*) {
                const auto result = request_handler_.goToMaxZoom(req->camera_id());
                if (result.isSuccess()) {
                    return Result<void>::success();
                }
                return Result<void>::error(result.error());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::SetAutoFocus(
        grpc::CallbackServerContext* context,
        const core::v1::SetAutoFocusRequest* request,
        google::protobuf::Empty* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::SetAutoFocusRequest* req, google::protobuf::Empty*) {
                return request_handler_.enableAutoFocus(req->camera_id(), req->enable());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GetAutoFocus(
        grpc::CallbackServerContext* context,
        const core::v1::GetAutoFocusRequest* request,
        core::v1::GetAutoFocusResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GetAutoFocusRequest* req, core::v1::GetAutoFocusResponse* resp) {
                const auto result = request_handler_.getAutoFocus(req->camera_id());
                if (result.isSuccess()) {
                    resp->set_enable(result.value());
                    return Result<void>::success();
                }
                return Result<void>::error(result.error());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::SetStabilization(
        grpc::CallbackServerContext* context,
        const core::v1::SetStabilizationRequest* request,
        google::protobuf::Empty* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::SetStabilizationRequest* req, google::protobuf::Empty*) {
                return request_handler_.stabilize(req->camera_id(), req->enable());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GetStabilization(
        grpc::CallbackServerContext* context,
        const core::v1::GetStabilizationRequest* request,
        core::v1::GetStabilizationResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GetStabilizationRequest* req, core::v1::GetStabilizationResponse* resp) {
                const auto result = request_handler_.getStabilization(req->camera_id());
                if (result.isSuccess()) {
                    resp->set_enable(result.value());
                    return Result<void>::success();
                }
                return Result<void>::error(result.error());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::SetVideoCapabilityState(
        grpc::CallbackServerContext* context,
        const core::v1::SetVideoCapabilityStateRequest* request,
        google::protobuf::Empty* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::SetVideoCapabilityStateRequest* req, google::protobuf::Empty*) {
                return request_handler_.SetVideoCapabilityState(req->camera_id(), req->capability(), req->enable());
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GetVideoCapabilities(
        grpc::CallbackServerContext* context,
        const core::v1::GetVideoCapabilitiesRequest* request,
        core::v1::GetVideoCapabilitiesResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GetVideoCapabilitiesRequest* req, core::v1::GetVideoCapabilitiesResponse* resp) {
                const auto result = request_handler_.getVideoCapabilities(req->camera_id());
                if (result.isError()) {
                    return Result<void>::error(result.error());
                }

                for (const auto& capability : result.value()) {
                    resp->add_capabilities(capability);
                }

                return Result<void>::success();
            });
    }

    grpc::ServerUnaryReactor* GrpcCallbackHandler::GetVideoCapabilityState(
        grpc::CallbackServerContext* context,
        const core::v1::GetVideoCapabilityStateRequest* request,
        core::v1::GetVideoCapabilityStateResponse* response) {
        return handleGrpcSyncRequest(context, request, response,
            [this](const core::v1::GetVideoCapabilityStateRequest* req, core::v1::GetVideoCapabilityStateResponse* resp) {
                const auto result = request_handler_.getVideoCapabilityState(req->camera_id(), req->capability());
                if (result.isError()) {
                    return Result<void>::error(result.error());
                }

                resp->set_enable(result.value());
                return Result<void>::success();
            });
    }
} // namespace service::api

