#pragma once

#include <cstdint>
#include <functional>
#include "nabo_face.h"

class WifiProvisionUi;
class NaboFace;

/**
 * Minimal desktop shell around XiaoZhi.
 * QDTech had full DesktopUI; Tab5 phase-1 keeps Main / Xiaozhi / Settings.
 */
class DesktopUi {
 public:
    using StateHandler = std::function<void(const char* state, const char* message)>;

    void Create(int w, int h);
    void ShowMain();
    void ShowXiaozhi();
    void ShowSettings();
    void ShowWifiProvision();

    void SetNaboFace(NaboFace* face) { face_ = face; }
    void SetWifiUi(WifiProvisionUi* ui) { wifi_ = ui; }

    void SetXiaozhiState(const char* state, const char* message, const char* emotion);
    void SetClock(const char* text);
    void SetNetworkStatus(const char* text);
    void Tick();

 private:
    enum class Page : uint8_t { kMain, kXiaozhi, kSettings, kWifi };
    void ShowPage(Page p);

    Page page_ = Page::kXiaozhi;
    NaboFace* face_ = nullptr;
    WifiProvisionUi* wifi_ = nullptr;
    void* root_ = nullptr;
    void* label_clock_ = nullptr;
    void* label_net_ = nullptr;
};
