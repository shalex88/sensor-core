#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <string>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "../../Mocks.h"
#include "api/rest/RestTransport.h"
#include "common/types/Result.h"

using namespace service;
using namespace testing;
using json = nlohmann::json;

namespace {
    constexpr uint16_t TEST_PORT = 18080;
    constexpr auto TEST_HOST = "127.0.0.1";
    constexpr auto SERVER_STARTUP_DELAY = std::chrono::milliseconds(80);

    struct HttpResponse {
        int status_code{};
        std::string body{};
    };

    HttpResponse sendHttpRequest(const std::string& method,
                                  const std::string& path,
                                  const std::string& request_body = "",
                                  uint16_t port = TEST_PORT) {
        const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            return {};
        }

        timeval tv{};
        tv.tv_sec = 2;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, TEST_HOST, &server_addr.sin_addr);

        if (connect(sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            close(sockfd);
            return {};
        }

        std::string request = method + " " + path + " HTTP/1.1\r\n";
        request += std::string{"Host: "} + TEST_HOST + "\r\n";
        request += "Connection: close\r\n";
        if (!request_body.empty()) {
            request += "Content-Type: application/json\r\n";
            request += "Content-Length: " + std::to_string(request_body.size()) + "\r\n";
        }
        request += "\r\n" + request_body;

        send(sockfd, request.data(), request.size(), 0);

        std::string response_str;
        std::array<char, 4096> buffer{};
        ssize_t n = 0;
        while ((n = recv(sockfd, buffer.data(), buffer.size() - 1, 0)) > 0) {
            response_str.append(buffer.data(), static_cast<size_t>(n));
        }
        close(sockfd);

        HttpResponse resp;
        if (const auto pos = response_str.find(' '); pos != std::string::npos) {
            resp.status_code = std::stoi(response_str.substr(pos + 1, 3));
        }
        if (const auto pos = response_str.find("\r\n\r\n"); pos != std::string::npos) {
            resp.body = response_str.substr(pos + 4);
        }
        return resp;
    }
} // namespace

class RestTransportTests : public Test {
protected:
    void SetUp() override {
        handler_mock_ = std::make_unique<RequestHandlerMock>();
        transport_ = std::make_unique<api::RestTransport>(*handler_mock_);
    }

    void TearDown() override {
        transport_->stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    void startServer() {
        ASSERT_TRUE(transport_->start(TEST_HOST, TEST_PORT).isSuccess());
        server_thread_ = std::thread([this] {
            const auto result = transport_->runLoop();
            EXPECT_TRUE(result.isSuccess());
        });
        std::this_thread::sleep_for(SERVER_STARTUP_DELAY);
    }

    std::unique_ptr<RequestHandlerMock> handler_mock_;
    std::unique_ptr<api::RestTransport> transport_;
    std::thread server_thread_;
};

// ─── Lifecycle tests ─────────────────────────────────────────────────────────

TEST_F(RestTransportTests, CreationSuccess) {
    ASSERT_NE(nullptr, transport_);
}

TEST_F(RestTransportTests, StartSuccess) {
    const auto result = transport_->start(TEST_HOST, TEST_PORT);
    ASSERT_TRUE(result.isSuccess());
}

TEST_F(RestTransportTests, StartWithEmptyServerFails) {
    const auto result = transport_->start("", TEST_PORT);
    ASSERT_TRUE(result.isError());
}

TEST_F(RestTransportTests, StartWithZeroPortFails) {
    const auto result = transport_->start(TEST_HOST, 0);
    ASSERT_TRUE(result.isError());
}

TEST_F(RestTransportTests, StartTwiceFails) {
    ASSERT_TRUE(transport_->start(TEST_HOST, TEST_PORT).isSuccess());
    ASSERT_TRUE(transport_->start(TEST_HOST, TEST_PORT).isError());
}

TEST_F(RestTransportTests, StopWhenNotStartedSucceeds) {
    ASSERT_TRUE(transport_->stop().isSuccess());
}

TEST_F(RestTransportTests, StopStartedServerSucceeds) {
    ASSERT_TRUE(transport_->start(TEST_HOST, TEST_PORT).isSuccess());
    ASSERT_TRUE(transport_->stop().isSuccess());
}

TEST_F(RestTransportTests, StopMultipleTimesSucceeds) {
    ASSERT_TRUE(transport_->start(TEST_HOST, TEST_PORT).isSuccess());
    ASSERT_TRUE(transport_->stop().isSuccess());
    ASSERT_TRUE(transport_->stop().isSuccess());
}

TEST_F(RestTransportTests, RunLoopWithoutStartFails) {
    ASSERT_TRUE(transport_->runLoop().isError());
}

TEST_F(RestTransportTests, RunLoopAfterStopFails) {
    ASSERT_TRUE(transport_->start(TEST_HOST, TEST_PORT).isSuccess());
    ASSERT_TRUE(transport_->stop().isSuccess());
    ASSERT_TRUE(transport_->runLoop().isError());
}

TEST_F(RestTransportTests, RunLoopWithRunningServerSucceeds) {
    ASSERT_TRUE(transport_->start(TEST_HOST, TEST_PORT).isSuccess());

    std::jthread server_thread([&] {
        ASSERT_TRUE(transport_->runLoop().isSuccess());
    });

    std::this_thread::sleep_for(SERVER_STARTUP_DELAY);
    ASSERT_TRUE(transport_->stop().isSuccess());
}

// ─── HTTP endpoint tests ──────────────────────────────────────────────────────

TEST_F(RestTransportTests, HealthEndpointReturns200) {
    startServer();

    const auto resp = sendHttpRequest("GET", "/api/v1/health");

    EXPECT_EQ(200, resp.status_code);
    const auto body = json::parse(resp.body);
    EXPECT_EQ("ok", body.at("status").get<std::string>());
}

TEST_F(RestTransportTests, OptionsRequestReturnsCorsHeaders) {
    startServer();

    const auto resp = sendHttpRequest("OPTIONS", "/api/v1/health");

    EXPECT_EQ(204, resp.status_code);
}

TEST_F(RestTransportTests, UnknownRouteReturns404) {
    startServer();

    const auto resp = sendHttpRequest("GET", "/api/v1/unknown");

    EXPECT_EQ(404, resp.status_code);
}

TEST_F(RestTransportTests, GetZoomReturnsZoomValue) {
    EXPECT_CALL(*handler_mock_, getZoom(1))
        .WillOnce(Return(Result<common::types::zoom>::success(42u)));

    startServer();

    const auto resp = sendHttpRequest("GET", "/api/v1/cameras/1/zoom");

    EXPECT_EQ(200, resp.status_code);
    const auto body = json::parse(resp.body);
    EXPECT_EQ(42u, body.at("zoom").get<uint32_t>());
}

TEST_F(RestTransportTests, GetZoomReturns500OnHandlerError) {
    EXPECT_CALL(*handler_mock_, getZoom(1))
        .WillOnce(Return(Result<common::types::zoom>::error("camera unavailable")));

    startServer();

    const auto resp = sendHttpRequest("GET", "/api/v1/cameras/1/zoom");

    EXPECT_EQ(500, resp.status_code);
}

TEST_F(RestTransportTests, PutZoomSetsValueAndResponds200) {
    EXPECT_CALL(*handler_mock_, setZoomAndGet(1, 55u))
        .WillOnce(Return(Result<common::types::zoom>::success(55u)));

    startServer();

    const auto resp = sendHttpRequest("PUT", "/api/v1/cameras/1/zoom", R"({"zoom":55})");

    EXPECT_EQ(200, resp.status_code);
    const auto body = json::parse(resp.body);
    EXPECT_EQ(55u, body.at("zoom").get<uint32_t>());
}

TEST_F(RestTransportTests, PutZoomWithMalformedJsonReturns400) {
    startServer();

    const auto resp = sendHttpRequest("PUT", "/api/v1/cameras/1/zoom", "not-json");

    EXPECT_EQ(400, resp.status_code);
}

TEST_F(RestTransportTests, GetFocusReturnsFocusValue) {
    EXPECT_CALL(*handler_mock_, getFocus(2))
        .WillOnce(Return(Result<common::types::focus>::success(30u)));

    startServer();

    const auto resp = sendHttpRequest("GET", "/api/v1/cameras/2/focus");

    EXPECT_EQ(200, resp.status_code);
    const auto body = json::parse(resp.body);
    EXPECT_EQ(30u, body.at("focus").get<uint32_t>());
}

TEST_F(RestTransportTests, GetAutoFocusReturnsState) {
    EXPECT_CALL(*handler_mock_, getAutoFocus(1))
        .WillOnce(Return(Result<bool>::success(true)));

    startServer();

    const auto resp = sendHttpRequest("GET", "/api/v1/cameras/1/autofocus");

    EXPECT_EQ(200, resp.status_code);
    const auto body = json::parse(resp.body);
    EXPECT_TRUE(body.at("enable").get<bool>());
}

TEST_F(RestTransportTests, PutAutoFocusEnablesAutofocus) {
    EXPECT_CALL(*handler_mock_, enableAutoFocusAndGet(1, true))
        .WillOnce(Return(Result<bool>::success(true)));

    startServer();

    const auto resp = sendHttpRequest("PUT", "/api/v1/cameras/1/autofocus", R"({"enable":true})");

    EXPECT_EQ(200, resp.status_code);
}

TEST_F(RestTransportTests, GetStabilizationReturnsState) {
    EXPECT_CALL(*handler_mock_, getStabilization(1))
        .WillOnce(Return(Result<bool>::success(false)));

    startServer();

    const auto resp = sendHttpRequest("GET", "/api/v1/cameras/1/stabilization");

    EXPECT_EQ(200, resp.status_code);
    const auto body = json::parse(resp.body);
    EXPECT_FALSE(body.at("enable").get<bool>());
}

TEST_F(RestTransportTests, GetStreamUrlBuildsCorrectUrl) {
    EXPECT_CALL(*handler_mock_, getStreamUrl(3))
        .WillOnce(Return(Result<std::string>::success("http://stream-service/camera3")));

    startServer();

    const auto resp = sendHttpRequest("GET", "/api/v1/cameras/3/stream/url");

    EXPECT_EQ(200, resp.status_code);
    const auto body = json::parse(resp.body);
    EXPECT_EQ("http://stream-service/camera3", body.at("url").get<std::string>());
}

TEST_F(RestTransportTests, PutZoomMinCallsGoToMinZoom) {
    EXPECT_CALL(*handler_mock_, goToMinZoomAndGet(1))
        .WillOnce(Return(Result<common::types::zoom>::success(0u)));

    startServer();

    const auto resp = sendHttpRequest("PUT", "/api/v1/cameras/1/zoom/min");

    EXPECT_EQ(200, resp.status_code);
    const auto body = json::parse(resp.body);
    EXPECT_EQ(0u, body.at("zoom").get<uint32_t>());
}

TEST_F(RestTransportTests, PutZoomMaxCallsGoToMaxZoom) {
    EXPECT_CALL(*handler_mock_, goToMaxZoomAndGet(1))
        .WillOnce(Return(Result<common::types::zoom>::success(100u)));

    startServer();

    const auto resp = sendHttpRequest("PUT", "/api/v1/cameras/1/zoom/max");

    EXPECT_EQ(200, resp.status_code);
    const auto body = json::parse(resp.body);
    EXPECT_EQ(100u, body.at("zoom").get<uint32_t>());
}
