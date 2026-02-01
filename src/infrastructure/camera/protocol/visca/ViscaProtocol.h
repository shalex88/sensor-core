#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <mutex>
#include <span>

#include "common/types/Result.h"

namespace service::infrastructure {
    class ITransport;

    class ViscaProtocol {
    public:
        struct ViscaPayload;

        struct ViscaTitleData {
            uint32_t vposition{};
            uint32_t hposition{};
            uint32_t color{};
            uint32_t blink{};
            std::array<std::byte, 20> title{};
        };

        explicit ViscaProtocol(std::unique_ptr<ITransport> transport);
        ~ViscaProtocol() noexcept;

        Result<void> setAddress();
        Result<void> clear();
        Result<std::string> getCameraInfo();
        Result<void> open() const;
        Result<void> close() const;
        Result<void> setPower(uint8_t power);
        Result<void> setKeylock(uint8_t power);
        Result<void> setCameraId(uint16_t id);
        Result<void> setZoomTele();
        Result<void> setZoomWide();
        Result<void> setZoomStop();
        Result<void> setZoomTeleSpeed(uint32_t speed); //TODO: uint8_t?
        Result<void> setZoomWideSpeed(uint32_t speed); //TODO: uint8_t?
        Result<void> setZoomValue(uint16_t zoom);
        Result<void> setZoomAndFocusValue(uint16_t zoom, uint16_t focus);
        Result<void> setDzoomValue(uint8_t value);
        Result<void> setDzoomLimit(uint8_t limit);
        Result<void> setDzoomMode(uint8_t power);
        Result<void> setFocusFar();
        Result<void> setFocusNear();
        Result<void> setFocusStop();
        Result<void> setFocusFarSpeed(uint32_t speed); //TODO: uint8_t?
        Result<void> setFocusNearSpeed(uint32_t speed); //TODO: uint8_t?
        Result<void> setFocusValue(uint16_t focus);
        Result<void> setFocusAuto(bool on);
        Result<void> setFocusOnePush();
        Result<void> setFocusInfinity();
        Result<void> setFocusAutosenseHigh();
        Result<void> setFocusAutosenseLow();
        Result<void> setFocusNearLimit(uint16_t limit);
        Result<void> setWhitebalMode(uint8_t mode);
        Result<void> setWhitebalOnePush();
        Result<void> setRgainUp();
        Result<void> setRgainDown();
        Result<void> setRgainReset();
        Result<void> setRgainValue(uint8_t value);
        Result<void> setBgainUp();
        Result<void> setBgainDown();
        Result<void> setBgainReset();
        Result<void> setBgainValue(uint8_t value);
        Result<void> setShutterUp();
        Result<void> setShutterDown();
        Result<void> setShutterReset();
        Result<void> setShutterValue(uint8_t value);
        Result<void> setIrisUp();
        Result<void> setIrisDown();
        Result<void> setIrisReset();
        Result<void> setIrisValue(uint8_t value);
        Result<void> setGainUp();
        Result<void> setGainDown();
        Result<void> setGainReset();
        Result<void> setGainValue(uint8_t value);
        Result<void> setBrightUp();
        Result<void> setBrightDown();
        Result<void> setBrightReset();
        Result<void> setBrightValue(uint16_t value); //TODO: uint8_t?
        Result<void> setApertureUp();
        Result<void> setApertureDown();
        Result<void> setApertureReset();
        Result<void> setApertureValue(uint8_t value);
        Result<void> setExpCompUp();
        Result<void> setExpCompDown();
        Result<void> setExpCompReset();
        Result<void> setExpCompValue(uint8_t value);
        Result<void> setExpCompPower(uint8_t power);
        Result<void> setAutoExpMode(uint8_t mode);
        Result<void> setSlowShutterAuto(uint8_t power);
        Result<void> setBacklightComp(bool on);
        Result<void> setZeroLuxShot(uint8_t power);
        Result<void> setIrLed(uint8_t power);
        Result<void> setWideMode(uint8_t mode);
        Result<void> setMirror(uint8_t power);
        Result<void> setFreeze(uint8_t power);
        Result<void> setPictureEffect(uint8_t mode);
        Result<void> setDigitalEffect(uint8_t mode);
        Result<void> setDigitalEffectLevel(uint8_t level);
        Result<void> setCamStabilizer(bool power);
        Result<void> memorySet(uint8_t channel);
        Result<void> memoryRecall(uint8_t channel);
        Result<void> memoryReset(uint8_t channel);
        Result<void> setDisplay(uint8_t power);
        Result<void> setDateTime(uint16_t year, uint16_t month, uint16_t day, uint16_t hour,
                                 uint16_t minute);
        Result<void> setDateDisplay(uint8_t power);
        Result<void> setTimeDisplay(uint8_t power);
        Result<void> setTitleDisplay(uint8_t power);
        Result<void> setTitleClear();
        Result<void> setTitleParams(const ViscaTitleData& title);
        Result<void> setTitle(const ViscaTitleData& title);
        Result<void> setIrreceiveOn();
        Result<void> setIrreceiveOff();
        Result<void> setIrreceiveOnoff();
        Result<void> setPanTiltUp(uint8_t pan_speed, uint8_t tilt_speed);
        Result<void> setPanTiltDown(uint8_t pan_speed, uint8_t tilt_speed);
        Result<void> setPanTiltLeft(uint8_t pan_speed, uint8_t tilt_speed);
        Result<void> setPanTiltRight(uint8_t pan_speed, uint8_t tilt_speed);
        Result<void> setPanTiltUpleft(uint8_t pan_speed, uint8_t tilt_speed);
        Result<void> setPanTiltUpright(uint8_t pan_speed, uint8_t tilt_speed);
        Result<void> setPanTiltDownleft(uint8_t pan_speed, uint8_t tilt_speed);
        Result<void> setPanTiltDownright(uint8_t pan_speed, uint8_t tilt_speed);
        Result<void> setPanTiltStop(uint8_t pan_speed, uint8_t tilt_speed);
        Result<void> setPanTiltAbsolutePosition(uint8_t pan_speed, uint8_t tilt_speed, uint16_t pan_position,
                                                uint16_t tilt_position);
        Result<void> setPanTiltRelativePosition(uint8_t pan_speed, uint8_t tilt_speed, uint16_t pan_position,
                                                uint16_t tilt_position);
        Result<void> setPanTiltHome();
        Result<void> setPanTiltReset();
        Result<void> setPanTiltLimitUpright(uint16_t pan_limit, uint16_t tilt_limit);
        Result<void> setPanTiltLimitDownleft(uint16_t pan_limit, uint16_t tilt_limit);
        Result<void> setPanTiltLimitDownleftClear();
        Result<void> setPanTiltLimitUprightClear();
        Result<void> setDatascreenOn();
        Result<void> setDatascreenOff();
        Result<void> setDatascreenOnoff();
        Result<void> setSpotAeOn();
        Result<void> setSpotAeOff();
        Result<void> setSpotAePosition(uint8_t x_position, uint8_t y_position);
        Result<uint8_t> getPower();
        Result<uint8_t> getDzoomValue();
        Result<uint8_t> getDzoomLimit();
        Result<uint16_t> getZoomValue();
        Result<bool> getFocusAuto();
        Result<uint16_t> getFocusValue();
        Result<uint8_t> getFocusAutoSense();
        Result<uint16_t> getFocusNearLimit();
        Result<uint8_t> getWhitebalMode();
        Result<uint8_t> getRgainValue();
        Result<uint8_t> getBgainValue();
        Result<uint8_t> getAutoExpMode();
        Result<uint8_t> getSlowShutterAuto();
        Result<uint8_t> getShutterValue();
        Result<uint8_t> getIrisValue();
        Result<uint8_t> getGainValue();
        Result<uint16_t> getBrightValue(); //TODO: uint8_t?
        Result<uint8_t> getExpCompPower();
        Result<uint8_t> getExpCompValue();
        Result<bool> getBacklightComp();
        Result<uint8_t> getApertureValue();
        Result<uint8_t> getZeroLuxShot();
        Result<uint8_t> getIrLed();
        Result<uint8_t> getWideMode();
        Result<uint8_t> getMirror();
        Result<uint8_t> getFreeze();
        Result<uint8_t> getPictureEffect();
        Result<uint8_t> getDigitalEffect();
        Result<uint16_t> getDigitalEffectLevel();
        Result<uint8_t> getMemory();
        Result<uint8_t> getDisplay();
        Result<uint16_t> getId();
        Result<uint8_t> getVideoSystem();
        Result<uint16_t> getPanTiltMode();
        Result<std::pair<uint8_t, uint8_t>> getPanTiltMaxspeed();
        Result<std::pair<uint16_t, uint16_t>> getPanTiltPosition();
        Result<uint8_t> getDatascreen();
        Result<void> setWideConLens(uint8_t power);
        Result<void> setAtModeOnOff();
        Result<void> setAtMode(uint8_t power);
        Result<void> setAtAeOnoff();
        Result<void> setAtAe(uint8_t power);
        Result<void> setAtAutozoomOnoff();
        Result<void> setAtAutozoom(uint8_t power);
        Result<void> setAtmdFrameDisplayOnOff();
        Result<void> setAtmdFrameDisplay(uint8_t power);
        Result<void> setAtFrameOffsetOnOff();
        Result<void> setAtFrameOffset(uint8_t power);
        Result<void> setAtmdStartStop();
        Result<void> setAtChase(uint8_t power);
        Result<void> setAtChaseNext();
        Result<void> setMdModeOnoff();
        Result<void> setMdMode(uint8_t power);
        Result<void> setMdFrame();
        Result<void> setMdDetect();
        Result<void> setAtEntry(uint8_t power);
        Result<void> setAtLostinfo();
        Result<void> setMdLostinfo();
        Result<void> setMdAdjustYlevel(uint8_t power);
        Result<void> setMdAdjustHuelevel(uint8_t power);
        Result<void> setMdAdjustSize(uint8_t power);
        Result<void> setMdAdjustDisptime(uint8_t power);
        Result<void> setMdAdjustRefmode(uint8_t power);
        Result<void> setMdAdjustReftime(uint8_t power);
        Result<void> setMdMeasureMode1OnOff();
        Result<void> setMdMeasureMode1(uint8_t power);
        Result<void> setMdMeasureMode2OnOff();
        Result<void> setMdMeasureMode2(uint8_t power);
        Result<uint8_t> getKeylock();
        Result<uint8_t> getWideConLens();
        Result<uint8_t> getAtmdMode();
        Result<uint16_t> getAtMode();
        Result<uint8_t> getAtEntry();
        Result<uint16_t> getMdMode();
        Result<uint8_t> getMdYlevel();
        Result<uint8_t> getMdHuelevel();
        Result<uint8_t> getMdSize();
        Result<uint8_t> getMdDisptime();
        Result<uint8_t> getMdRefmode();
        Result<uint8_t> getMdReftime();
        Result<void> setRegister(uint8_t reg_num, uint8_t reg_val);
        Result<uint8_t> getRegister(uint8_t reg_num);

    private:
        std::unique_ptr<ITransport> transport_;
        uint8_t broadcast_{};
        uint8_t cam_address_{};
        std::mutex mutex_;
        Result<ViscaPayload> writeRead(ViscaPayload& payload);
        std::vector<std::byte> encode(std::span<const std::byte> payload);
        Result<void> write(ViscaPayload& payload);
        Result<ViscaPayload> read() const;
    };
} // namespace service::infrastructure
