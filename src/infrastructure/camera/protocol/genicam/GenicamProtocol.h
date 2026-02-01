#pragma once

#include "common/types/Result.h"
#include "infrastructure/camera/protocol/genicam/include/GenApi/GenApi.h"

namespace service::infrastructure {
    class FpgaTransport;

    class GenicamProtocol final {
    public:
        explicit GenicamProtocol(std::unique_ptr<GENAPI_NAMESPACE::IPort> transport);
        ~GenicamProtocol();
        Result<void> open();
        Result<void> close() const;
        Result<void> startAcquisition() const;
        Result<void> stopAcquisition() const;
        Result<std::string> getDeviceVendorName() const;
        Result<std::string> getDeviceModelName() const;
        Result<std::string> getDeviceManufacturerInfo() const;
        Result<std::string> getDeviceFirmwareVersion() const;

    private:
        Result<std::string> getString(const std::string& feature) const;
        bool setFloat(const std::string& feature, double value) const;
        bool setInteger(const std::string& feature, int64_t value) const;
        bool setBoolean(const std::string& feature, bool value) const;
        bool setEnum(const std::string& feature, const std::string& value) const;
        bool executeCommand(const std::string& feature) const;

        std::unique_ptr<GENAPI_NAMESPACE::IPort> transport_{};
        GENAPI_NAMESPACE::CNodeMapRef node_map_{};
    };
}