#pragma once

#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "api/ITransport.h"
#include "api/IRequestHandler.h"
#include "core/ICore.h"
#include "common/types/CameraCapabilities.h"

using namespace service;
using namespace testing;

class TransportMock: public api::ITransport {
public:
    MOCK_METHOD(Result<void>, start, (const std::string&), (override));
    MOCK_METHOD(Result<void>, stop, (), (override));
    MOCK_METHOD(Result<void>, runLoop, (), (override));
};

class RequestHandlerMock: public api::IRequestHandler {
public:
    MOCK_METHOD(Result<void>, start, (), (override));
    MOCK_METHOD(Result<void>, stop, (), (override));
    MOCK_METHOD(bool, isRunning, (), (const, override));
    MOCK_METHOD(Result<void>, setZoom, (uint32_t, common::types::zoom), (const, override));
    MOCK_METHOD(Result<common::types::zoom>, getZoom, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, setFocus, (uint32_t, common::types::focus), (const, override));
    MOCK_METHOD(Result<common::types::focus>, getFocus, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, enableAutoFocus, (uint32_t, bool), (const, override));
    MOCK_METHOD(Result<bool>, getAutoFocus, (uint32_t), (const, override));
    MOCK_METHOD(Result<common::types::info>, getInfo, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, goToMinZoom, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, goToMaxZoom, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, stabilize, (uint32_t, bool), (const, override));
    MOCK_METHOD(Result<bool>, getStabilization, (uint32_t), (const, override));
    MOCK_METHOD(Result<common::capabilities::CapabilityList>, getCapabilities, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, SetVideoCapabilityState, (uint32_t, const std::string&, bool), (const, override));
    MOCK_METHOD(Result<std::vector<std::string>>, getVideoCapabilities, (uint32_t), (const, override));
    MOCK_METHOD(Result<bool>, getVideoCapabilityState, (uint32_t, const std::string&), (const, override));
};

class CoreMock: public core::ICore {
public:
    MOCK_METHOD(Result<void>, start, (), (override));
    MOCK_METHOD(Result<void>, stop, (), (override));

    MOCK_METHOD(Result<void>, setZoom, (uint32_t, common::types::zoom), (const, override));
    MOCK_METHOD(Result<common::types::zoom>, getZoom, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, goToMinZoom, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, goToMaxZoom, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, setFocus, (uint32_t, common::types::focus), (const, override));
    MOCK_METHOD(Result<common::types::focus>, getFocus, (uint32_t), (const, override));
    MOCK_METHOD(Result<common::types::info>, getInfo, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, enableAutoFocus, (uint32_t, bool), (const, override));
    MOCK_METHOD(Result<bool>, getAutoFocus, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, stabilize, (uint32_t, bool), (const, override));
    MOCK_METHOD(Result<bool>, getStabilization, (uint32_t), (const, override));
    MOCK_METHOD(Result<common::capabilities::CapabilityList>, getCapabilities, (uint32_t), (const, override));
    MOCK_METHOD(Result<void>, SetVideoCapabilityState, (uint32_t, const std::string&, bool), (const, override));
    MOCK_METHOD(Result<std::vector<std::string>>, getVideoCapabilities, (uint32_t), (const, override));
    MOCK_METHOD(Result<bool>, getVideoCapabilityState, (uint32_t, const std::string&), (const, override));

};