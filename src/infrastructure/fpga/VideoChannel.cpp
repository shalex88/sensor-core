#include "VideoChannel.h"

#include "infrastructure/camera/transport/mmio/RegisterImplUio.h"

namespace service::infrastructure {
    namespace {
        Result<void> configureChannel(int channel_num) {
            LOG_DEBUG("Configuring video channel {}...", channel_num);
            {
                const auto global_reg = std::make_unique<RegisterImplUio>("/dev/uio12");
                if (const auto result = global_reg->set(0x800B0034, 0x00000000); result.isError()) {
                    return Result<void>::error("Failed to disable video channel");
                }
            }
            {
                const auto mipi_reg = std::make_unique<RegisterImplUio>("/dev/uio14");
                if (const auto result = mipi_reg->set(0x800D0000, 0x00000000); result.isError()) {
                    return Result<void>::error("Failed to disable MIPI RX");
                }
            }
            {
                const auto test_pattern_reg = std::make_unique<RegisterImplUio>("/dev/uio6");
                if (const auto result = test_pattern_reg->set(0x80050008, 0x0000000F); result.isError()) {
                    return Result<void>::error("Failed to disable test pattern generator");
                }
                if (const auto result = test_pattern_reg->set(0x80050070, 0x00000780); result.isError()) {
                    return Result<void>::error("Failed to set test pattern frame height");
                }
                if (const auto result = test_pattern_reg->set(0x80050074, 0x00000438); result.isError()) {
                    return Result<void>::error("Failed to set test pattern line length");
                }
                if (const auto result = test_pattern_reg->set(0x80050078, 0x00000468); result.isError()) {
                    return Result<void>::error("Failed to set test pattern active video width");
                }
                if (const auto result = test_pattern_reg->set(0x8005007C, 0x0065B9AA); result.isError()) {
                    return Result<void>::error("Failed to set test pattern color bar pattern");
                }
                if (const auto result = test_pattern_reg->set(0x8005009C, 0x00000001); result.isError()) {
                    return Result<void>::error("Failed to enable test pattern generator");
                }
            }
            {
                const auto mux_reg = std::make_unique<RegisterImplUio>("/dev/uio7");
                if (const auto result = mux_reg->set(0x80060068, 0x00000000); result.isError()) {
                    return Result<void>::error("Failed to disable camera video");
                }
                if (const auto result = mux_reg->set(0x80060048, 0x00000001); result.isError()) {
                    return Result<void>::error("Failed to set camera video input");
                }
            }
            {
                const auto global_reg = std::make_unique<RegisterImplUio>("/dev/uio12");
                if (const auto result = global_reg->set(0x800B0014, 0x00000001); result.isError()) {
                    return Result<void>::error("Failed to reset video channel");
                }
                if (const auto result = global_reg->set(0x800B0024, 0x00000960); result.isError()) {
                    return Result<void>::error("Failed to set video channel width");
                }
            }
            {
                const auto mipi_reg = std::make_unique<RegisterImplUio>("/dev/uio14");
                if (const auto result = mipi_reg->set(0x800D0040, 0x00000438); result.isError()) {
                    return Result<void>::error("Failed to set MIPI RX line length");
                }
                if (const auto result = mipi_reg->set(0x800D0004, 0x0000E01B); result.isError()) {
                    return Result<void>::error("Failed to set MIPI RX frame height");
                }
                if (const auto result = mipi_reg->set(0x800D0000, 0x00000001); result.isError()) {
                    return Result<void>::error("Failed to enable MIPI RX");
                }
            }
            {
                const auto global_reg = std::make_unique<RegisterImplUio>("/dev/uio12");
                if (const auto result = global_reg->set(0x800B0034, 0x00000001); result.isError()) {
                    return Result<void>::error("Failed to enable video channel");
                }
            }
            {
                const auto mux_reg = std::make_unique<RegisterImplUio>("/dev/uio7");
                if (const auto result = mux_reg->set(0x80060068, 0x00000001); result.isError()) {
                    return Result<void>::error("Failed to enable camera video");
                }
            }

            return Result<void>::success();
        }
    } // unnamed namespace

    VideoChannel::VideoChannel(int channel_num) {
        if (const auto result = configureChannel(channel_num); result.isError()) {
            throw std::runtime_error("Failed to configure video channel: " + result.error());
        }
    }

    VideoChannel::~VideoChannel() noexcept = default;
} // namespace service::infrastructure