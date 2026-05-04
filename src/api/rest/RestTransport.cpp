#include "RestTransport.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>
#include <regex>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "api/IRequestHandler.h"
#include "common/logger/Logger.h"

namespace service::api {
    using json = nlohmann::json;

    namespace {
        constexpr auto API_BASE = "/api/v1";
        constexpr auto CONTENT_TYPE_JSON = "application/json";
        constexpr auto SERVER_NAME = "sensor-core";
        constexpr auto CORS_ALLOW_ORIGIN = "*";
        constexpr auto CORS_ALLOW_METHODS = "GET, PUT, OPTIONS";
        constexpr auto CORS_ALLOW_HEADERS = "Content-Type, Authorization";
        constexpr auto CORS_MAX_AGE = "86400";

        struct ConnectionData {
            std::string body{};
        };

        std::regex makeApiRegex(const std::string& suffix_pattern) {
            return std::regex(std::string{API_BASE} + suffix_pattern);
        }

        const std::regex STREAM_URL_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/stream/url)");
        const std::regex CAMERA_INFO_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/info)");
        const std::regex CAMERA_CAPABILITIES_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/capabilities)");
        const std::regex CAMERA_ZOOM_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/zoom)");
        const std::regex CAMERA_ZOOM_MIN_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/zoom/min)");
        const std::regex CAMERA_ZOOM_MAX_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/zoom/max)");
        const std::regex CAMERA_FOCUS_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/focus)");
        const std::regex CAMERA_AUTOFOCUS_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/autofocus)");
        const std::regex CAMERA_STABILIZATION_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/stabilization)");
        const std::regex VIDEO_CAPABILITIES_ROUTE_REGEX = makeApiRegex(R"(/cameras/(\d+)/video/capabilities)");
        const std::regex VIDEO_CAPABILITY_STATE_ROUTE_REGEX =
            makeApiRegex(R"(/cameras/(\d+)/video/capabilities/(\w+))");


        uint32_t parseCameraId(const std::string& path, const std::regex& route_regex) {
            if (std::smatch match; std::regex_match(path, match, route_regex)) {
                return std::stoul(match[1].str());
            }
            throw std::invalid_argument("Invalid camera_id in path");
        }

        std::pair<uint32_t, std::string> parseCameraIdAndCapability(const std::string& path) {
            if (std::smatch match; std::regex_match(path, match, VIDEO_CAPABILITY_STATE_ROUTE_REGEX)) {
                return {std::stoul(match[1].str()), match[2].str()};
            }
            throw std::invalid_argument("Invalid capability in path");
        }

        uint32_t parseJsonUint(const json& obj, const std::string& key) {
            if (!obj.contains(key) || !obj[key].is_number_unsigned()) {
                throw std::invalid_argument("Missing or invalid unsigned integer field: " + key);
            }
            return obj[key].get<uint32_t>();
        }

        bool parseJsonBool(const json& obj, const std::string& key) {
            if (!obj.contains(key) || !obj[key].is_boolean()) {
                throw std::invalid_argument("Missing or invalid boolean field: " + key);
            }
            return obj[key].get<bool>();
        }
    } // unnamed namespace

    RestTransport::RestTransport(IRequestHandler& request_handler) : request_handler_(request_handler) {}

    RestTransport::~RestTransport() {
        if (stop().isError()) {
            LOG_ERROR("RestTransport failed to stop gracefully");
        }
    }

    Result<void> RestTransport::start(const std::string& server, uint16_t port) {
        LOG_DEBUG("Starting RestTransport on {}:{}", server, port);

        if (server.empty()) {
            return Result<void>::error("Server cannot be empty");
        }

        if (port == 0) {
            return Result<void>::error("Port cannot be zero");
        }

        if (is_running_.load()) {
            return Result<void>::error("Server is already running");
        }

        host_ = server;
        port_ = port;
        is_running_.store(true);
        LOG_INFO("REST transport configured for {}:{}", server, port);
        return Result<void>::success();
    }

    Result<void> RestTransport::stop() {
        if (!is_running_.load()) {
            return Result<void>::success();
        }

        LOG_DEBUG("Stopping RestTransport...");
        {
            std::lock_guard lock{run_mutex_};
            is_running_.store(false);
        }
        run_cv_.notify_all();
        return Result<void>::success();
    }

    Result<void> RestTransport::runLoop() {
        if (!is_running_.load()) {
            return Result<void>::error("Server is not running");
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        if (inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) != 1) {
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
        }

        auto* daemon = MHD_start_daemon(
            MHD_USE_INTERNAL_POLLING_THREAD,
            port_,
            nullptr, nullptr,
            &RestTransport::accessHandler, this,
            MHD_OPTION_SOCK_ADDR, static_cast<sockaddr*>(static_cast<void*>(&addr)),
            MHD_OPTION_NOTIFY_COMPLETED, &RestTransport::requestCompleted, nullptr,
            MHD_OPTION_END);

        if (daemon == nullptr) {
            is_running_.store(false);
            return Result<void>::error("Failed to start MHD daemon on " + host_ + ":" + std::to_string(port_));
        }

        LOG_INFO("REST server started on {}:{}", host_, port_);
        {
            std::unique_lock lock{run_mutex_};
            run_cv_.wait(lock, [this] { return !is_running_.load(); });
        }

        MHD_stop_daemon(daemon);
        LOG_INFO("REST server stopped");
        return Result<void>::success();
    }

    MHD_Result RestTransport::accessHandler(void* cls,
                                              MHD_Connection* connection,
                                              const char* url,
                                              const char* method,
                                              const char* /*version*/,
                                              const char* upload_data,
                                              size_t* upload_data_size,
                                              void** con_cls) {
        if (*con_cls == nullptr) {
            *con_cls = new ConnectionData{};
            return MHD_YES;
        }

        auto* conn_data = static_cast<ConnectionData*>(*con_cls);

        if (*upload_data_size > 0) {
            conn_data->body.append(upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        }

        auto* self = static_cast<RestTransport*>(cls);
        return self->handleRequest(connection, method, url, conn_data->body);
    }

    void RestTransport::requestCompleted(void* /*cls*/,
                                          MHD_Connection* /*connection*/,
                                          void** con_cls,
                                          MHD_RequestTerminationCode /*toe*/) {
        delete static_cast<ConnectionData*>(*con_cls);
        *con_cls = nullptr;
    }

    MHD_Result RestTransport::handleRequest(MHD_Connection* connection,
                                              const std::string& method,
                                              const std::string& path,
                                              const std::string& body) const {
        auto add_cors_headers = [](MHD_Response* response) {
            MHD_add_response_header(response, "Access-Control-Allow-Origin", CORS_ALLOW_ORIGIN);
            MHD_add_response_header(response, "Access-Control-Allow-Methods", CORS_ALLOW_METHODS);
            MHD_add_response_header(response, "Access-Control-Allow-Headers", CORS_ALLOW_HEADERS);
            MHD_add_response_header(response, "Access-Control-Max-Age", CORS_MAX_AGE);
        };

        auto send_response = [&](unsigned int status_code,
                                  const std::string& body_str,
                                  const std::string& content_type = CONTENT_TYPE_JSON) -> MHD_Result {
            MHD_Response* response = nullptr;
            if (body_str.empty()) {
                response = MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
            } else {
                response = MHD_create_response_from_buffer(
                    body_str.size(),
                    const_cast<void*>(static_cast<const void*>(body_str.data())),
                    MHD_RESPMEM_MUST_COPY);
            }
            if (response == nullptr) {
                return MHD_NO;
            }
            MHD_add_response_header(response, MHD_HTTP_HEADER_SERVER, SERVER_NAME);
            if (!content_type.empty()) {
                MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, content_type.c_str());
            }
            add_cors_headers(response);
            const auto ret = MHD_queue_response(connection, status_code, response);
            MHD_destroy_response(response);
            return ret;
        };

        auto send_json = [&](unsigned int status_code, const json& json_body) -> MHD_Result {
            return send_response(status_code, json_body.dump());
        };

        auto send_empty = [&](unsigned int status_code) -> MHD_Result {
            return send_response(status_code, "", "");
        };

        auto send_error = [&](unsigned int status_code,
                               const std::string& msg,
                               const std::string& details = "") -> MHD_Result {
            json error_body = {{"error", msg}, {"message", msg}};
            if (!details.empty()) {
                error_body["details"] = details;
            }
            return send_json(status_code, error_body);
        };

        const auto health_path = std::string{API_BASE} + "/health";

        if (method == "OPTIONS") {
            return send_empty(MHD_HTTP_NO_CONTENT);
        }

        try {
            if (method == "GET" && path == health_path) {
                return send_json(MHD_HTTP_OK, json{{"status", "ok"}});
            }

            if (method == "GET" && std::regex_match(path, STREAM_URL_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, STREAM_URL_ROUTE_REGEX);
                const auto stream_url = "http://" + host_ + ":8889/camera" + std::to_string(camera_id);
                return send_json(MHD_HTTP_OK, json{{"url", stream_url}});
            }

            if (method == "GET" && std::regex_match(path, CAMERA_INFO_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_INFO_ROUTE_REGEX);
                const auto result = request_handler_.getInfo(camera_id);
                if (result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"info", result.value()}});
            }

            if (method == "GET" && std::regex_match(path, CAMERA_CAPABILITIES_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_CAPABILITIES_ROUTE_REGEX);
                const auto result = request_handler_.getCapabilities(camera_id);
                if (result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                json capabilities_array = json::array();
                for (const auto& capability : result.value()) {
                    capabilities_array.push_back(capability);
                }
                return send_json(MHD_HTTP_OK, json{{"capabilities", capabilities_array}});
            }

            if (method == "GET" && std::regex_match(path, CAMERA_ZOOM_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_ZOOM_ROUTE_REGEX);
                const auto result = request_handler_.getZoom(camera_id);
                if (result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"zoom", result.value()}});
            }

            if (method == "PUT" && std::regex_match(path, CAMERA_ZOOM_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_ZOOM_ROUTE_REGEX);
                const auto json_body = json::parse(body);
                const auto zoom_value = parseJsonUint(json_body, "zoom");
                if (const auto result = request_handler_.setZoom(camera_id, zoom_value); result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"zoom", zoom_value}});
            }

            if (method == "PUT" && std::regex_match(path, CAMERA_ZOOM_MIN_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_ZOOM_MIN_ROUTE_REGEX);
                if (const auto result = request_handler_.goToMinZoom(camera_id); result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"message", "Zoom moved to minimum"}});
            }

            if (method == "PUT" && std::regex_match(path, CAMERA_ZOOM_MAX_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_ZOOM_MAX_ROUTE_REGEX);
                if (const auto result = request_handler_.goToMaxZoom(camera_id); result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"message", "Zoom moved to maximum"}});
            }

            if (method == "GET" && std::regex_match(path, CAMERA_FOCUS_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_FOCUS_ROUTE_REGEX);
                const auto result = request_handler_.getFocus(camera_id);
                if (result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"focus", result.value()}});
            }

            if (method == "PUT" && std::regex_match(path, CAMERA_FOCUS_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_FOCUS_ROUTE_REGEX);
                const auto json_body = json::parse(body);
                const auto focus_value = parseJsonUint(json_body, "focus");
                if (const auto result = request_handler_.setFocus(camera_id, focus_value); result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"focus", focus_value}});
            }

            if (method == "GET" && std::regex_match(path, CAMERA_AUTOFOCUS_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_AUTOFOCUS_ROUTE_REGEX);
                const auto result = request_handler_.getAutoFocus(camera_id);
                if (result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"enable", result.value()}});
            }

            if (method == "PUT" && std::regex_match(path, CAMERA_AUTOFOCUS_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_AUTOFOCUS_ROUTE_REGEX);
                const auto json_body = json::parse(body);
                const auto enable = parseJsonBool(json_body, "enable");
                if (const auto result = request_handler_.enableAutoFocus(camera_id, enable); result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"enable", enable}});
            }

            if (method == "GET" && std::regex_match(path, CAMERA_STABILIZATION_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_STABILIZATION_ROUTE_REGEX);
                const auto result = request_handler_.getStabilization(camera_id);
                if (result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"enable", result.value()}});
            }

            if (method == "PUT" && std::regex_match(path, CAMERA_STABILIZATION_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, CAMERA_STABILIZATION_ROUTE_REGEX);
                const auto json_body = json::parse(body);
                const auto enable = parseJsonBool(json_body, "enable");
                if (const auto result = request_handler_.stabilize(camera_id, enable); result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"enable", enable}});
            }

            if (method == "GET" && std::regex_match(path, VIDEO_CAPABILITIES_ROUTE_REGEX)) {
                const auto camera_id = parseCameraId(path, VIDEO_CAPABILITIES_ROUTE_REGEX);
                const auto result = request_handler_.getVideoCapabilities(camera_id);
                if (result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                json capabilities_array = json::array();
                for (const auto& cap : result.value()) {
                    capabilities_array.push_back(cap);
                }
                return send_json(MHD_HTTP_OK, json{{"capabilities", capabilities_array}});
            }

            if (method == "GET" && std::regex_match(path, VIDEO_CAPABILITY_STATE_ROUTE_REGEX)) {
                const auto [camera_id, capability] = parseCameraIdAndCapability(path);
                const auto result = request_handler_.getVideoCapabilityState(camera_id, capability);
                if (result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"enable", result.value()}});
            }

            if (method == "PUT" && std::regex_match(path, VIDEO_CAPABILITY_STATE_ROUTE_REGEX)) {
                const auto [camera_id, capability] = parseCameraIdAndCapability(path);
                const auto json_body = json::parse(body);
                const auto enable = parseJsonBool(json_body, "enable");
                if (const auto result = request_handler_.SetVideoCapabilityState(camera_id, capability, enable);
                    result.isError()) {
                    return send_error(MHD_HTTP_INTERNAL_SERVER_ERROR, "Internal server error", result.error());
                }
                return send_json(MHD_HTTP_OK, json{{"enable", enable}});
            }

            return send_error(MHD_HTTP_NOT_FOUND, "Not found", "Route not found");
        } catch (const std::exception& e) {
            return send_error(MHD_HTTP_BAD_REQUEST, "Bad request", e.what());
        }
    }
} // namespace service::api
