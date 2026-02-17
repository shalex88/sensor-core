#include "RestTransport.h"

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <stdexcept>

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
            std::regex camera_regex(R"(/cameras/(\d+)/)");
            std::smatch match;
            if (std::regex_search(path, match, camera_regex)) {
                return std::stoul(match[1].str());
            }
            throw std::invalid_argument("Invalid camera_id in path");
        }

        void sendJsonResponse(httplib::Response& res, int status_code, const json& body) {
            res.set_header("Content-Type", CONTENT_TYPE_JSON);
            res.status = status_code;
            res.set_content(body.dump(), CONTENT_TYPE_JSON);
        }

        void sendErrorResponse(
            httplib::Response& res,
            int status_code,
            const std::string& error_message,
            const std::string& details = "") {
            json error_body = {
                {"error", error_message},
                {"message", error_message}
            };
            if (!details.empty()) {
                error_body["details"] = details;
            }
            sendJsonResponse(res, status_code, error_body);
        }

        std::string parseJsonString(const json& obj, const std::string& key) {
            if (!obj.contains(key) || !obj[key].is_string()) {
                throw std::invalid_argument("Missing or invalid string field: " + key);
            }
            return obj[key].get<std::string>();
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

    RestTransport::RestTransport(IRequestHandler& request_handler)
        : request_handler_(request_handler), server_(std::make_unique<httplib::Server>()) {
    }

    RestTransport::~RestTransport() {
        if (stop().isError()) {
            LOG_ERROR("RestTransport failed to stop gracefully");
        }
    }

    Result<void> RestTransport::start(const std::string& server_address) {
        LOG_DEBUG("Starting RestTransport on {}", server_address);

        const auto colon_pos = server_address.find(':');
        if (colon_pos == std::string::npos) {
            return Result<void>::error("Invalid server address format");
        }

        const auto host = server_address.substr(0, colon_pos);
        const auto port_str = server_address.substr(colon_pos + 1);

        int port;
        try {
            port = std::stoi(port_str);
        } catch (const std::exception& e) {
            return Result<void>::error("Invalid port number: " + port_str);
        }

        if (!server_) {
            return Result<void>::error("Server instance is null");
        }

        setupRoutes();

        // Configure the server to be ready to listen but don't start listening yet
        server_->set_address_family(AF_INET);
        // Store configuration for later use in runLoop
        server_address_ = server_address;

        is_running_ = true;
        LOG_INFO("REST transport configured for {}", server_address);
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

        LOG_DEBUG("REST server runLoop started on {}", server_address_);

        const auto colon_pos = server_address_.find(':');
        const auto host = server_address_.substr(0, colon_pos);
        const auto port_str = server_address_.substr(colon_pos + 1);
        const int port = std::stoi(port_str);

        if (!server_->listen(host.c_str(), port)) {
            return Result<void>::error("Failed to listen on " + server_address_);
        }

        LOG_INFO("REST server listening on {}", server_address_);
        return Result<void>::success();
    }

    void RestTransport::setupRoutes() {
        server_->set_default_headers({
            {"Access-Control-Allow-Origin", CORS_ALLOW_ORIGIN},
            {"Access-Control-Allow-Methods", CORS_ALLOW_METHODS},
            {"Access-Control-Allow-Headers", CORS_ALLOW_HEADERS},
            {"Access-Control-Max-Age", CORS_MAX_AGE}
        });

        server_->Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
            res.status = 204;
        });

        // GET /api/v1/health - Health check
        server_->Get(std::string(API_BASE) + "/health", [this](const httplib::Request&, httplib::Response& res) {
            json response = {
                {"status", "ok"}
            };
            sendJsonResponse(res, 200, response);
        });

        // GET /api/v1/cameras/{cameraId}/stream/url - Get HLS stream URL
        server_->Get(std::string(API_BASE) + "/cameras/(\\d+)/stream/url",
            [this](const httplib::Request& req, httplib::Response& res) {
                try {
                    const auto camera_id = extractCameraId(req.path);
                    json response = {
                        {"url", "http://camera.local/hls/stream.m3u8"}
                    };
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
                    json response = {
                        {"info", result.value()}
                    };
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

                    json response = {
                        {"capabilities", capabilities_array}
                    };
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
                    json response = {
                        {"zoom", result.value()}
                    };
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

                    const auto result = request_handler_.setZoom(camera_id, zoom_value);
                    if (result.isError()) {
                        sendErrorResponse(res, 500, "Internal server error", result.error());
                        return;
                    }
                    json response = {
                        {"zoom", zoom_value}
                    };
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
                    const auto result = request_handler_.goToMinZoom(camera_id);
                    if (result.isError()) {
                        sendErrorResponse(res, 500, "Internal server error", result.error());
                        return;
                    }
                    json response = {
                        {"message", "Zoom moved to minimum"}
                    };
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
                    const auto result = request_handler_.goToMaxZoom(camera_id);
                    if (result.isError()) {
                        sendErrorResponse(res, 500, "Internal server error", result.error());
                        return;
                    }
                    json response = {
                        {"message", "Zoom moved to maximum"}
                    };
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
                    json response = {
                        {"focus", result.value()}
                    };
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

                    const auto result = request_handler_.setFocus(camera_id, focus_value);
                    if (result.isError()) {
                        sendErrorResponse(res, 500, "Internal server error", result.error());
                        return;
                    }
                    json response = {
                        {"focus", focus_value}
                    };
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
                    json response = {
                        {"enable", result.value()}
                    };
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

                    const auto result = request_handler_.enableAutoFocus(camera_id, enable);
                    if (result.isError()) {
                        sendErrorResponse(res, 500, "Internal server error", result.error());
                        return;
                    }
                    json response = {
                        {"enable", enable}
                    };
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
                    json response = {
                        {"enable", result.value()}
                    };
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

                    const auto result = request_handler_.stabilize(camera_id, enable);
                    if (result.isError()) {
                        sendErrorResponse(res, 500, "Internal server error", result.error());
                        return;
                    }
                    json response = {
                        {"enable", enable}
                    };
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

                    json response = {
                        {"capabilities", capabilities_array}
                    };
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

                    std::regex capability_regex(R"(/video/capabilities/(\w+))");
                    std::smatch match;
                    std::string capability;
                    if (std::regex_search(req.path, match, capability_regex)) {
                        capability = match[1].str();
                    } else {
                        throw std::invalid_argument("Invalid capability in path");
                    }

                    const auto result = request_handler_.getVideoCapabilityState(camera_id, capability);
                    if (result.isError()) {
                        sendErrorResponse(res, 500, "Internal server error", result.error());
                        return;
                    }
                    json response = {
                        {"enable", result.value()}
                    };
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

                    std::regex capability_regex(R"(/video/capabilities/(\w+))");
                    std::smatch match;
                    std::string capability;
                    if (std::regex_search(req.path, match, capability_regex)) {
                        capability = match[1].str();
                    } else {
                        throw std::invalid_argument("Invalid capability in path");
                    }

                    const auto body = json::parse(req.body);
                    const auto enable = parseJsonBool(body, "enable");

                    const auto result = request_handler_.SetVideoCapabilityState(camera_id, capability, enable);
                    if (result.isError()) {
                        sendErrorResponse(res, 500, "Internal server error", result.error());
                        return;
                    }
                    json response = {
                        {"enable", enable}
                    };
                    sendJsonResponse(res, 200, response);
                } catch (const std::exception& e) {
                    sendErrorResponse(res, 400, "Bad request", e.what());
                }
            });
    }
} // namespace service::api
