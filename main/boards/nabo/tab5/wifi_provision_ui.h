#pragma once

#include <cstdint>
#include <functional>

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef enum {
    NABO_WIFI_IDLE = 0,
    NABO_WIFI_SCANNING,
    NABO_WIFI_LIST,
    NABO_WIFI_INPUT_PSK,
    NABO_WIFI_CONNECTING,
    NABO_WIFI_SUCCESS,
    NABO_WIFI_ERROR,
} nabo_wifi_ui_state_t;

typedef struct {
    char ssid[33];
    int8_t rssi;
    bool locked;
    bool is_11ax;
} nabo_wifi_ap_t;

/**
 * Settings → Wi-Fi provisioning UI (device-side, Tab5 touchscreen).
 * SoftAP phone portal remains a fallback in Board::GetBoardType flow.
 */
class WifiProvisionUi {
 public:
    using ConnectFn = std::function<void(const char* ssid, const char* password)>;
    using ScanFn = std::function<int(nabo_wifi_ap_t* out, int max_count)>;
    using StatusFn = std::function<void(const char* line)>;

    void Create(void* lv_parent);
    void Show();
    void Hide();
    nabo_wifi_ui_state_t state() const { return state_; }

    void SetScanner(ScanFn scan);
    void SetConnector(ConnectFn connect);
    void SetStatusSink(StatusFn status);

    /** Complete an asynchronous connection request from the network task. */
    void SetConnectionResult(bool connected, const char* detail = nullptr);

    /** Drive UI: call from LVGL timer / Application tick. */
    void Tick();

 private:
    void OnScanClicked();
    void OnApSelected(int index);
    void OnPskConfirmed(const char* psk);
    void OnConnectClicked();
    void SetState(nabo_wifi_ui_state_t s);

    nabo_wifi_ui_state_t state_ = NABO_WIFI_IDLE;
    nabo_wifi_ap_t aps_[20] = {};
    int ap_count_ = 0;
    int selected_ = -1;
    char ssid_[33] = {};
    char psk_[65] = {};
    ScanFn scan_;
    ConnectFn connect_;
    StatusFn status_;

    lv_obj_t* root_ = nullptr;
    lv_obj_t* status_label_ = nullptr;
    lv_obj_t* list_ = nullptr;
    lv_obj_t* scan_button_ = nullptr;
    lv_obj_t* connect_button_ = nullptr;
    lv_obj_t* password_textarea_ = nullptr;
    lv_obj_t* keyboard_ = nullptr;
    lv_obj_t* spinner_ = nullptr;
};
