#include "nabo_tab5_board.h"
#include "config.h"

#include <esp_log.h>

#define TAG "NaboTab5Board"

// Placeholder board bring-up. Wire real drivers from:
// - m5stack-tab5 / M5Tab5-UserDemo (ST7121 init + PI4IOE)
// - espressif/m5stack_tab5 BSP
// - qdtech board pattern for DesktopUI service wiring

NaboTab5Board::NaboTab5Board() {
    InitializeI2c();
    InitializePi4ioe();
    InitializeDisplaySt7121();
    InitializeTouch();
    InitializeAudio();
    InitializeWifiProvision();
    InitializeNaboFace();
    ESP_LOGI(TAG, "NABO Tab5 board ready (panel=ST7121 %dx%d)", DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

void NaboTab5Board::InitializeI2c() {
    // i2c_master_bus on SDA=31 SCL=32 — shared codec/touch/sensors
}

void NaboTab5Board::InitializePi4ioe() {
    // PI4IOE5V6408 @ 0x43/0x44: SPK_EN, WLAN_PWR_EN, LCD_RST, TP_RST, USB5V, CHG_EN
}

void NaboTab5Board::InitializeDisplaySt7121() {
#if NABO_PANEL_ST7121
    // MIPI-DSI + ST7121 TDDI init (see ST7123 protocol doc / M5GFX path)
    // LCD_RST via PI4IOE E1.P4: low = push-pull/open-drain, high = input pull-up
#else
    // Legacy ILI9881C + GT911
#endif
}

void NaboTab5Board::InitializeTouch() {
    // ST7121 I2C 0x55 → lv_indev (preferred over hand-rolled hit tests)
}

void NaboTab5Board::InitializeAudio() {
    // Tab5AudioCodec: ES8388 + ES7210 AEC
}

void NaboTab5Board::InitializeWifiProvision() {
    wifi_ui_.SetScanner([](nabo_wifi_ap_t* out, int max_count) {
        // WifiStation::Scan → fill out[]; return count
        (void)out;
        (void)max_count;
        return 0;
    });
    wifi_ui_.SetConnector([](const char* ssid, const char* password) {
        // WifiStation::Connect + NVS persist
        (void)ssid;
        (void)password;
        return false;
    });
}

void NaboTab5Board::InitializeNaboFace() {
    nabo_face_.Create(nullptr);  // attach to DesktopUI Xiaozhi page root later
}

AudioCodec* NaboTab5Board::GetAudioCodec() {
    return nullptr;  // static Tab5AudioCodec
}

Display* NaboTab5Board::GetDisplay() {
    return nullptr;
}

Backlight* NaboTab5Board::GetBacklight() {
    return nullptr;
}

int NaboTab5Board::GetBatteryPercent() {
    return -1;
}

bool NaboTab5Board::IsCharging() {
    return false;
}

// DECLARE_BOARD(NaboTab5Board);
