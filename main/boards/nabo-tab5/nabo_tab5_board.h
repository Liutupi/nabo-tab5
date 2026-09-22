#pragma once

#include "wifi_board.h"
#include "wifi_provision_ui.h"
#include "nabo_face.h"

/**
 * NABO Tab5 board (ST7121).
 * Composition follows QDTech pattern: Board owns hardware, DesktopUI owns pages,
 * Services (WifiProvisionUi / NaboFace) are board-local product layer.
 */
class NaboTab5Board : public WifiBoard {
 public:
    NaboTab5Board();

    AudioCodec* GetAudioCodec() override;
    Display* GetDisplay() override;
    Backlight* GetBacklight() override;

    NaboFace* GetNaboFace() { return &nabo_face_; }
    WifiProvisionUi* GetWifiProvisionUi() { return &wifi_ui_; }

    /** Battery percent via INA226 heuristic; -1 if unknown. */
    int GetBatteryPercent();
    bool IsCharging();

 private:
    void InitializeI2c();
    void InitializePi4ioe();
    void InitializeDisplaySt7121();
    void InitializeTouch();
    void InitializeAudio();
    void InitializeWifiProvision();
    void InitializeNaboFace();

    void* i2c_bus_ = nullptr;
    NaboFace nabo_face_;
    WifiProvisionUi wifi_ui_;
};
