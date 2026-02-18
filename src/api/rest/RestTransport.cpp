#include "RestTransport.h"

#include <httplib.h>
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
        constexpr auto CORS_ALLOW_ORIGIN = "*";
        constexpr auto CORS_ALLOW_METHODS = "GET, PUT, OPTIONS";
        constexpr auto CORS_ALLOW_HEADERS = "Content-Type, Authorization";
        constexpr auto CORS_MAX_AGE = "86400";

        uint32_t extractCameraId(const std::string& path) {
            const std::regex camera_regex(R"(/cameras/(\d+)/)");
            if (std::smatch match; std::regex_search(path, match, camera_regex)) {
                return std::stoul(match[1].str());
            }
            throw std::invalid_argument("Invalid camera_id in path");
        }

        void sendJsonResponse(httplib::Response& res, int status_code, const json& body) {
            res.set_header("Content-Type", CONTENT_TYPE_JSON);
            res.status = status_code;
            res.set_content(body.dump(), CONTENT_TYPE_JSON);
        }

        void sendErrorResponse(httplib::Response& res, const int status_code, const std::string& error_message,
                               const std::string& details = "") {
            json error_body = {{"error", error_message}, {"message", error_message}};
            if (!details.empty()) {
                error_body["details"] = details;
            }
            sendJsonResponse(res, status_code, error_body);
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

    RestTransport::RestTransport(IRequestHandler& request_handler) : request_handler_(request_handler),
                                                                     server_(std::make_unique<httplib::Server>()) {
    }

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

        if (!server_) {
            return Result<void>::error("Server instance is null");
        }

        setupRoutes();

        // Configure the server to be ready to listen but don't start listening yet
        server_->set_address_family(AF_INET);
        // Store configuration for later use in runLoop
        host_ = server;
        port_ = port;

        is_running_ = true;
        LOG_INFO("REST transport configured for {}:{}", server, port);
        return Result<void>::success();
    }

    Result<void> RestTransport::stop() {
        if (!is_running_) {
            return Result<void>::success();
        }

        LOG_DEBUG("Stopping RestTransport...");
        if (server_) {
            server_->stop();
        }
        is_running_ = false;
        LOG_DEBUG("RestTransport stopped");
        return Result<void>::success();
    }

    Result<void> RestTransport::runLoop() {
        if (!is_running_) {
            return Result<void>::error("Server is not running");
        }

        LOG_DEBUG("REST server runLoop started on {}:{}", host_, port_);

        if (!server_->listen(host_, port_)) {
            return Result<void>::error("Failed to listen on " + host_ + ":" + std::to_string(port_));
        }

        LOG_INFO("REST server listening on {}:{}", host_, port_);
        return Result<void>::success();
    }

    void RestTransport::setupRoutes() const {
        server_->set_default_headers({
            {"Access-Control-Allow-Origin", CORS_ALLOW_ORIGIN}, {"Access-Control-Allow-Methods", CORS_ALLOW_METHODS},
            {"Access-Control-Allow-Headers", CORS_ALLOW_HEADERS}, {"Access-Control-Max-Age", CORS_MAX_AGE}
        });

        server_->Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
            res.status = 204;
        });

        // GET /api/v1/health - Health check
        server_->Get(std::string(API_BASE) + "/health", [](const httplib::Request&, httplib::Response& res) {
            const json response = {
                {"status", "ok"} //TODO: pass to core
            };
            sendJsonResponse(res, 200, response);
        });

        // GET /api/v1/cameras/{cameraId}/stream/url - Get HLS stream URL
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/stream/url",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto stream_url = "http://" + host_ + ":8888" + "/camera" + std::to_string(camera_id)
                                 + "/index.m3u8";
                             const json response = {{"url", stream_url}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // GET /api/v1/cameras/{cameraId}/info - Get camera info
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/info",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto result = request_handler_.getInfo(camera_id);
                             if (result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"info", result.value()}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // GET /api/v1/cameras/{cameraId}/capabilities - Get camera capabilities
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/capabilities",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto result = request_handler_.getCapabilities(camera_id);
                             if (result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }

                             json capabilities_array = json::array();
                             for (const auto& cap : result.value()) {
                                 capabilities_array.push_back(cap);
                             }

                             const json response = {{"capabilities", capabilities_array}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // GET /api/v1/cameras/{cameraId}/zoom - Get current zoom
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/zoom",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto result = request_handler_.getZoom(camera_id);
                             if (result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"zoom", result.value()}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // PUT /api/v1/cameras/{cameraId}/zoom - Set zoom
        server_->Put(std::string(API_BASE) + "/cameras/(\\d+)/zoom",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto body = json::parse(req.body);
                             const auto zoom_value = parseJsonUint(body, "zoom");

                             if (const auto result = request_handler_.setZoom(camera_id, zoom_value); result.
                                 isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"zoom", zoom_value}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // PUT /api/v1/cameras/{cameraId}/zoom/min - Move zoom to minimum
        server_->Put(std::string(API_BASE) + "/cameras/(\\d+)/zoom/min",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             if (const auto result = request_handler_.goToMinZoom(camera_id); result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"message", "Zoom moved to minimum"}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // PUT /api/v1/cameras/{cameraId}/zoom/max - Move zoom to maximum
        server_->Put(std::string(API_BASE) + "/cameras/(\\d+)/zoom/max",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             if (const auto result = request_handler_.goToMaxZoom(camera_id); result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"message", "Zoom moved to maximum"}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // GET /api/v1/cameras/{cameraId}/focus - Get current focus
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/focus",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto result = request_handler_.getFocus(camera_id);
                             if (result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"focus", result.value()}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // PUT /api/v1/cameras/{cameraId}/focus - Set focus
        server_->Put(std::string(API_BASE) + "/cameras/(\\d+)/focus",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto body = json::parse(req.body);
                             const auto focus_value = parseJsonUint(body, "focus");

                             if (const auto result = request_handler_.setFocus(camera_id, focus_value); result.
                                 isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"focus", focus_value}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // GET /api/v1/cameras/{cameraId}/autofocus - Get autofocus state
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/autofocus",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto result = request_handler_.getAutoFocus(camera_id);
                             if (result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"enable", result.value()}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // PUT /api/v1/cameras/{cameraId}/autofocus - Enable/disable autofocus
        server_->Put(std::string(API_BASE) + "/cameras/(\\d+)/autofocus",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto body = json::parse(req.body);
                             const auto enable = parseJsonBool(body, "enable");

                             if (const auto result = request_handler_.enableAutoFocus(camera_id, enable); result.
                                 isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"enable", enable}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // GET /api/v1/cameras/{cameraId}/stabilization - Get stabilization state
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/stabilization",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto result = request_handler_.getStabilization(camera_id);
                             if (result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"enable", result.value()}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // PUT /api/v1/cameras/{cameraId}/stabilization - Enable/disable stabilization
        server_->Put(std::string(API_BASE) + "/cameras/(\\d+)/stabilization",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto body = json::parse(req.body);
                             const auto enable = parseJsonBool(body, "enable");

                             if (const auto result = request_handler_.stabilize(camera_id, enable); result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"enable", enable}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // GET /api/v1/cameras/{cameraId}/video/capabilities - Get enabled video capabilities
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/video/capabilities",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);
                             const auto result = request_handler_.getVideoCapabilities(camera_id);
                             if (result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }

                             json capabilities_array = json::array();
                             for (const auto& cap : result.value()) {
                                 capabilities_array.push_back(cap);
                             }

                             const json response = {{"capabilities", capabilities_array}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // GET /api/v1/cameras/{cameraId}/video/capabilities/{capability} - Get video capability state
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/video/capabilities/(\\w+)",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);

                             const std::regex capability_regex(R"(/video/capabilities/(\w+))");
                             std::string capability;
                             if (std::smatch match; std::regex_search(req.path, match, capability_regex)) {
                                 capability = match[1].str();
                             } else {
                                 throw std::invalid_argument("Invalid capability in path");
                             }

                             const auto result = request_handler_.getVideoCapabilityState(camera_id, capability);
                             if (result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"enable", result.value()}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });

        // PUT /api/v1/cameras/{cameraId}/video/capabilities/{capability} - Enable/disable video capability
        server_->Put(std::string(API_BASE) + "/cameras/(\\d+)/video/capabilities/(\\w+)",
                     [this](const httplib::Request& req, httplib::Response& res) {
                         try {
                             const auto camera_id = extractCameraId(req.path);

                             const std::regex capability_regex(R"(/video/capabilities/(\w+))");
                             std::string capability;
                             if (std::smatch match; std::regex_search(req.path, match, capability_regex)) {
                                 capability = match[1].str();
                             } else {
                                 throw std::invalid_argument("Invalid capability in path");
                             }

                             const auto body = json::parse(req.body);
                             const auto enable = parseJsonBool(body, "enable");

                             if (const auto result = request_handler_.SetVideoCapabilityState(
                                 camera_id, capability, enable); result.isError()) {
                                 sendErrorResponse(res, 500, "Internal server error", result.error());
                                 return;
                             }
                             const json response = {{"enable", enable}};
                             sendJsonResponse(res, 200, response);
                         } catch (const std::exception& e) {
                             sendErrorResponse(res, 400, "Bad request", e.what());
                         }
                     });
    }
} // namespace service::api
