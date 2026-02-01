#include "AdimecCamera.h"

#include "common/logger/Logger.h"
#include "infrastructure/camera/protocol/genicam/GenicamProtocol.h"
#include "infrastructure/camera/protocol/itl/ItlProtocol.h"

namespace service::infrastructure {
    AdimecCamera::AdimecCamera(std::unique_ptr<GenicamProtocol> camera_protocol, std::unique_ptr<ItlProtocol> lens_protocol) :
        camera_protocol_(std::move(camera_protocol)), lens_protocol_(std::move(lens_protocol)) {
    }

    Result<void> AdimecCamera::open() {
        if (camera_protocol_->open().isError()) {
            return Result<void>::error("Failed to connect to Adimec camera");
        }
        if (lens_protocol_) {
            if (lens_protocol_->open().isError()) {
                return Result<void>::error("Failed to connect to Adimec lens");
            }
        } else {
            LOG_WARN("No lens endpoint provided. Operating without lens control.");
        }
        return Result<void>::success();
    }

    Result<void> AdimecCamera::close() {
        if (camera_protocol_->close().isError()) {
            return Result<void>::error("Failed to disconnect from Adimec camera");
        }
        if (lens_protocol_) {
            if (lens_protocol_->close().isError()) {
                return Result<void>::error("Failed to disconnect from Adimec lens");
            }
        }
        return Result<void>::success();
    }

    Result<common::types::info> AdimecCamera::getInfo() const {
        std::string info;

        if (const auto vendor = camera_protocol_->getDeviceVendorName(); vendor.isSuccess()) {
            info += "Vendor: " + vendor.value() + "";
        }

        if (const auto model = camera_protocol_->getDeviceModelName(); model.isSuccess()) {
            info += "Model: " + model.value() + "";
        }

        if (const auto manufacturer_info = camera_protocol_->getDeviceManufacturerInfo(); manufacturer_info.isSuccess()) {
            info += "Manufacturer Info: " + manufacturer_info.value() + " ";
        }

        if (const auto firmware = camera_protocol_->getDeviceFirmwareVersion(); firmware.isSuccess()) {
            info += "Firmware Version: " + firmware.value();
        }

        if (info.empty()) {
            return Result<common::types::info>::error("Failed to retrieve camera information");
        }

        return Result<common::types::info>::success(info);
    }
} // namespace service::infrastructure