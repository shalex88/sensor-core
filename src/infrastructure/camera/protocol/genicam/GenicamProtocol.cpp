#include "GenicamProtocol.h"

namespace service::infrastructure {
    GenicamProtocol::GenicamProtocol(std::unique_ptr<GENAPI_NAMESPACE::IPort> transport)
        : transport_(std::move(transport)) {
        if (!transport_) {
            throw std::invalid_argument("Transport port cannot be null");
        }
    }

    GenicamProtocol::~GenicamProtocol() {
        if (close().isError()) {
            LOG_ERROR("Failed to close GenICam protocol");
        }
    }

    Result<void> GenicamProtocol::open() {
        try {
            node_map_._LoadXMLFromFile(
                "/home/shalex/dev/projects/camera-service/src/infrastructure/camera/protocol/genicam/TMX5x.xml");
            node_map_._Connect(transport_.get());
        } catch (const std::exception& e) {
            return Result<void>::error(e.what());
        }

        return startAcquisition();
    }

    Result<void> GenicamProtocol::close() const {
        return stopAcquisition();
    }

    Result<void> GenicamProtocol::startAcquisition() const {
        if (!executeCommand("AcquisitionStart")) {
            return Result<void>::error("Failed to start acquisition");
        }

        return Result<void>::success();
    }

    Result<void> GenicamProtocol::stopAcquisition() const {
        if (!executeCommand("AcquisitionStop")) {
            return Result<void>::error("Failed to stop acquisition");
        }

        return Result<void>::success();
    }

    Result<std::string> GenicamProtocol::getDeviceVendorName() const {
        return getString("DeviceVendorName");
    }

    Result<std::string> GenicamProtocol::getDeviceModelName() const {
        return getString("DeviceModelName");
    }

    Result<std::string> GenicamProtocol::getDeviceManufacturerInfo() const {
        return getString("DeviceManufacturerInfo");
    }

    Result<std::string> GenicamProtocol::getDeviceFirmwareVersion() const {
        return getString("DeviceFirmwareVersion");
    }

    Result<std::string> GenicamProtocol::getString(const std::string& feature) const {
        try {
            const GENAPI_NAMESPACE::CStringPtr node = node_map_._GetNode(feature.c_str());
            if (!GENAPI_NAMESPACE::IsReadable(node)) {
                return Result<std::string>::error("Feature '" + feature + "' is not readable");
            }
            return Result<std::string>::success(std::string(node->GetValue()));
        } catch (...) {
            return Result<std::string>::error("Failed to get " + feature);
        }
    }

    bool GenicamProtocol::setFloat(const std::string& feature, const double value) const {
        try {
            const GENAPI_NAMESPACE::CFloatPtr node = node_map_._GetNode(feature.c_str());
            if (!GENAPI_NAMESPACE::IsWritable(node)) {
                return false;
            }
            node->SetValue(value);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool GenicamProtocol::setInteger(const std::string& feature, const int64_t value) const {
        try {
            const GENAPI_NAMESPACE::CIntegerPtr node = node_map_._GetNode(feature.c_str());
            if (!GENAPI_NAMESPACE::IsWritable(node)) {
                return false;
            }
            node->SetValue(value);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool GenicamProtocol::setBoolean(const std::string& feature, const bool value) const {
        try {
            const GENAPI_NAMESPACE::CBooleanPtr node = node_map_._GetNode(feature.c_str());
            if (!GENAPI_NAMESPACE::IsWritable(node)) {
                return false;
            }
            node->SetValue(value);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool GenicamProtocol::setEnum(const std::string& feature, const std::string& value) const {
        try {
            const GENAPI_NAMESPACE::CEnumerationPtr node = node_map_._GetNode(feature.c_str());
            if (!GENAPI_NAMESPACE::IsWritable(node)) {
                return false;
            }
            const GENAPI_NAMESPACE::CEnumEntryPtr entry = node->GetEntryByName(value.c_str());
            if (!GENAPI_NAMESPACE::IsAvailable(entry)) {
                return false;
            }
            node->SetIntValue(entry->GetValue());
            return true;
        } catch (...) {
            return false;
        }
    }

    bool GenicamProtocol::executeCommand(const std::string& feature) const {
        try {
            const GENAPI_NAMESPACE::CCommandPtr node = node_map_._GetNode(feature.c_str());
            if (!GENAPI_NAMESPACE::IsWritable(node)) {
                return false;
            }
            node->Execute();
            return true;
        } catch (...) {
            return false;
        }
    }
}