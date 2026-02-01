#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "api/ITransport.h"
#include "api/IRequestHandler.h"
#include "core/ICore.h"
#include "infrastructure/camera/hal/ICamera.h"
#include "infrastructure/camera/hal/ICameraHw.h"
#include "infrastructure/camera/transport/mmio/IRegisterImpl.h"
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
    MOCK_METHOD(Result<void>, setZoom, (common::types::zoom), (const, override));
    MOCK_METHOD(Result<common::types::zoom>, getZoom, (), (const, override));
    MOCK_METHOD(Result<void>, setFocus, (common::types::focus), (const, override));
    MOCK_METHOD(Result<common::types::focus>, getFocus, (), (const, override));
    MOCK_METHOD(Result<void>, enableAutoFocus, (bool), (const, override));
    MOCK_METHOD(Result<common::types::info>, getInfo, (), (const, override));
    MOCK_METHOD(Result<void>, goToMinZoom, (), (const, override));
    MOCK_METHOD(Result<void>, goToMaxZoom, (), (const, override));
    MOCK_METHOD(Result<void>, stabilize, (bool), (const, override));
    MOCK_METHOD(Result<common::capabilities::CapabilityList>, getCapabilities, (), (const, override));
};

class CoreMock: public core::ICore {
public:
    MOCK_METHOD(Result<void>, start, (), (override));
    MOCK_METHOD(Result<void>, stop, (), (override));

    MOCK_METHOD(Result<void>, setZoom, (common::types::zoom), (const, override));
    MOCK_METHOD(Result<common::types::zoom>, getZoom, (), (const, override));
    MOCK_METHOD(Result<void>, goToMinZoom, (), (const, override));
    MOCK_METHOD(Result<void>, goToMaxZoom, (), (const, override));
    MOCK_METHOD(Result<void>, setFocus, (common::types::focus), (const, override));
    MOCK_METHOD(Result<common::types::focus>, getFocus, (), (const, override));
    MOCK_METHOD(Result<common::types::info>, getInfo, (), (const, override));
    MOCK_METHOD(Result<void>, enableAutoFocus, (bool), (const, override));
    MOCK_METHOD(Result<void>, stabilize, (bool), (const, override));
    MOCK_METHOD(Result<common::capabilities::CapabilityList>, getCapabilities, (), (const, override));
};

class MockCameraHal : public infrastructure::ICamera {
public:
    MOCK_METHOD(Result<void>, open, (), (override));
    MOCK_METHOD(Result<void>, close, (), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));

    // IZoomCapable implementation
    MOCK_METHOD(Result<void>, setZoom, (common::types::zoom), (const, override));
    MOCK_METHOD(Result<common::types::zoom>, getZoom, (), (const, override));
    MOCK_METHOD(common::types::ZoomRange, getZoomLimits, (), (const, override));

    // IFocusCapable implementation
    MOCK_METHOD(Result<void>, setFocus, (common::types::focus), (const, override));
    MOCK_METHOD(Result<common::types::focus>, getFocus, (), (const, override));
    MOCK_METHOD(common::types::FocusRange, getFocusLimits, (), (const, override));

    // IAutoFocusCapable implementation
    MOCK_METHOD(Result<void>, enableAutoFocus, (bool), (const, override));

    // IInfoCapable implementation
    MOCK_METHOD(Result<common::types::info>, getInfo, (), (const, override));

    // IStabilizationCapable implementation
    MOCK_METHOD(Result<void>, stabilize, (bool), (const, override));

    // Capability enumeration
    MOCK_METHOD(Result<common::capabilities::CapabilityList>, getCapabilities, (), (const, override));
};

class MockCameraHw: public infrastructure::ICameraHw,
                     public common::capabilities::IZoomCapable,
                     public common::capabilities::IFocusCapable,
                     public common::capabilities::IAutoFocusCapable,
                     public common::capabilities::IStabilizeCapable,
                     public common::capabilities::IInfoCapable {
public:
    MockCameraHw() {
        // Set up default behavior for zoom and focus limits to prevent constructor validation failures
        const common::types::ZoomRange default_zoom_limits{.min = 0, .max = 1000};
        const common::types::FocusRange default_focus_limits{.min = 0, .max = 1000};

        ON_CALL(*this, getZoomLimits())
            .WillByDefault(Return(default_zoom_limits));
        ON_CALL(*this, getFocusLimits())
            .WillByDefault(Return(default_focus_limits));
    }

    MOCK_METHOD(Result<void>, open, (), (override));
    MOCK_METHOD(Result<void>, close, (), (override));

    // IZoomCapable implementation
    MOCK_METHOD(Result<void>, setZoom, (common::types::zoom), (const, override));
    MOCK_METHOD(Result<common::types::zoom>, getZoom, (), (const, override));
    MOCK_METHOD(common::types::ZoomRange, getZoomLimits, (), (const, override));

    // IFocusCapable implementation
    MOCK_METHOD(Result<void>, setFocus, (common::types::focus), (const, override));
    MOCK_METHOD(Result<common::types::focus>, getFocus, (), (const, override));
    MOCK_METHOD(common::types::FocusRange, getFocusLimits, (), (const, override));

    // IAutoFocusCapable implementation
    MOCK_METHOD(Result<void>, enableAutoFocus, (bool), (const, override));

    // IInfoCapable implementation
    MOCK_METHOD(Result<common::types::info>, getInfo, (), (const, override));

    // IStabilizationCapable implementation
    MOCK_METHOD(Result<void>, stabilize, (bool), (const, override));
};

class MockRegisterImpl: public infrastructure::IRegisterImpl {
public:
    MOCK_METHOD(Result<void>, set, (uint32_t address, uint32_t value), (override));
    MOCK_METHOD(Result<uint32_t>, get, (uint32_t address), (const, override));
};