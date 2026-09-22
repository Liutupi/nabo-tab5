#include "wifi_provision_ui.h"

#include <esp_log.h>
#include <stdio.h>
#include <string.h>

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#define TAG "WifiProvUI"

struct WifiProvisionUiWidgets {
    lv_obj_t* root = nullptr;
    lv_obj_t* status = nullptr;
    lv_obj_t* list = nullptr;
    lv_obj_t* btn_scan = nullptr;
    lv_obj_t* btn_connect = nullptr;
    lv_obj_t* ta_psk = nullptr;
    lv_obj_t* kb = nullptr;
    lv_obj_t* spinner = nullptr;
};

static WifiProvisionUiWidgets g_w;

static const char* StateText(nabo_wifi_ui_state_t s) {
    switch (s) {
        case NABO_WIFI_IDLE: return "点「扫描」查找 Wi-Fi 6 / 2.4G 网络";
        case NABO_WIFI_SCANNING: return "正在扫描…";
        case NABO_WIFI_LIST: return "选择网络，开放网络将直接连接";
        case NABO_WIFI_INPUT_PSK: return "输入密码后点「连接」";
        case NABO_WIFI_CONNECTING: return "正在连接…";
        case NABO_WIFI_SUCCESS: return "已连接";
        case NABO_WIFI_ERROR: return "连接失败，请重试";
        default: return "";
    }
}

void WifiProvisionUi::Create(void* lv_parent) {
    auto* parent = static_cast<lv_obj_t*>(lv_parent);
    if (!parent) return;

    g_w.root = lv_obj_create(parent);
    lv_obj_set_size(g_w.root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(g_w.root, 16, 0);

    g_w.status = lv_label_create(g_w.root);
    lv_label_set_text(g_w.status, StateText(state_));
    lv_obj_align(g_w.status, LV_ALIGN_TOP_MID, 0, 0);

    g_w.btn_scan = lv_button_create(g_w.root);
    lv_obj_set_size(g_w.btn_scan, 160, 56);
    lv_obj_align(g_w.btn_scan, LV_ALIGN_TOP_LEFT, 0, 40);
    auto* lab = lv_label_create(g_w.btn_scan);
    lv_label_set_text(lab, "扫描网络");
    lv_obj_center(lab);
    lv_obj_add_event_cb(g_w.btn_scan, [](lv_event_t* e) {
        auto* self = static_cast<WifiProvisionUi*>(lv_event_get_user_data(e));
        if (self) self->OnScanClicked();
    }, LV_EVENT_CLICKED, this);

    g_w.list = lv_list_create(g_w.root);
    lv_obj_set_size(g_w.list, lv_pct(55), 420);
    lv_obj_align(g_w.list, LV_ALIGN_TOP_LEFT, 0, 120);

    g_w.ta_psk = lv_textarea_create(g_w.root);
    lv_obj_set_size(g_w.ta_psk, lv_pct(40), 56);
    lv_obj_align(g_w.ta_psk, LV_ALIGN_TOP_RIGHT, 0, 120);
    lv_textarea_set_one_line(g_w.ta_psk, true);
    lv_textarea_set_password_mode(g_w.ta_psk, true);
    lv_textarea_set_placeholder_text(g_w.ta_psk, "Wi-Fi 密码");
    lv_obj_add_flag(g_w.ta_psk, LV_OBJ_FLAG_HIDDEN);

    g_w.btn_connect = lv_button_create(g_w.root);
    lv_obj_set_size(g_w.btn_connect, 160, 56);
    lv_obj_align(g_w.btn_connect, LV_ALIGN_TOP_RIGHT, 0, 200);
    auto* cl = lv_label_create(g_w.btn_connect);
    lv_label_set_text(cl, "连接");
    lv_obj_center(cl);
    lv_obj_add_flag(g_w.btn_connect, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(g_w.btn_connect, [](lv_event_t* e) {
        auto* self = static_cast<WifiProvisionUi*>(lv_event_get_user_data(e));
        if (!self || !g_w.ta_psk) return;
        self->OnPskConfirmed(lv_textarea_get_text(g_w.ta_psk));
    }, LV_EVENT_CLICKED, this);

    lv_obj_add_flag(g_w.root, LV_OBJ_FLAG_HIDDEN);
}

void WifiProvisionUi::Show() {
    if (g_w.root) lv_obj_clear_flag(g_w.root, LV_OBJ_FLAG_HIDDEN);
    SetState(NABO_WIFI_IDLE);
}

void WifiProvisionUi::Hide() {
    if (g_w.root) lv_obj_add_flag(g_w.root, LV_OBJ_FLAG_HIDDEN);
}

void WifiProvisionUi::SetScanner(ScanFn scan) { scan_ = std::move(scan); }
void WifiProvisionUi::SetConnector(ConnectFn connect) { connect_ = std::move(connect); }
void WifiProvisionUi::SetStatusSink(StatusFn status) { status_ = std::move(status); }

void WifiProvisionUi::SetState(nabo_wifi_ui_state_t s) {
    state_ = s;
    if (g_w.status) lv_label_set_text(g_w.status, StateText(s));
    const bool show_psk = (s == NABO_WIFI_INPUT_PSK);
    const bool show_connect = show_psk || (s == NABO_WIFI_CONNECTING);
    if (g_w.ta_psk) {
        if (show_psk) lv_obj_clear_flag(g_w.ta_psk, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_w.ta_psk, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_w.btn_connect) {
        if (show_psk) lv_obj_clear_flag(g_w.btn_connect, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_w.btn_connect, LV_OBJ_FLAG_HIDDEN);
    }
    (void)show_connect;
    ESP_LOGI(TAG, "state=%d", static_cast<int>(s));
}

void WifiProvisionUi::OnScanClicked() {
    SetState(NABO_WIFI_SCANNING);
    ap_count_ = 0;
    selected_ = -1;
    if (g_w.list) lv_obj_clean(g_w.list);
    if (scan_) ap_count_ = scan_(aps_, 20);
    if (g_w.list) {
        for (int i = 0; i < ap_count_; ++i) {
            char line[80];
            snprintf(line, sizeof(line), "%s  %ddBm%s%s", aps_[i].ssid, aps_[i].rssi,
                     aps_[i].locked ? "  [加密]" : "", aps_[i].is_11ax ? "  Wi-Fi6" : "");
            auto* btn = lv_list_add_button(g_w.list, nullptr, line);
            lv_obj_set_user_data(btn, (void*)(intptr_t)i);
            lv_obj_add_event_cb(btn, [](lv_event_t* e) {
                auto* self = static_cast<WifiProvisionUi*>(lv_event_get_user_data(e));
                auto idx = (int)(intptr_t)lv_obj_get_user_data(lv_event_get_current_target_obj(e));
                if (self) self->OnApSelected(idx);
            }, LV_EVENT_CLICKED, this);
        }
    }
    SetState(NABO_WIFI_LIST);
}

void WifiProvisionUi::OnApSelected(int index) {
    if (index < 0 || index >= ap_count_) return;
    selected_ = index;
    snprintf(ssid_, sizeof(ssid_), "%s", aps_[index].ssid);
    if (aps_[index].locked) {
        SetState(NABO_WIFI_INPUT_PSK);
    } else {
        psk_[0] = 0;
        SetState(NABO_WIFI_CONNECTING);
        OnConnectClicked();
    }
}

void WifiProvisionUi::OnPskConfirmed(const char* psk) {
    snprintf(psk_, sizeof(psk_), "%s", psk ? psk : "");
    SetState(NABO_WIFI_CONNECTING);
    OnConnectClicked();
}

void WifiProvisionUi::OnConnectClicked() {
    const bool ok = connect_ ? connect_(ssid_, psk_) : false;
    SetState(ok ? NABO_WIFI_SUCCESS : NABO_WIFI_ERROR);
    if (status_) status_(ok ? ssid_ : "connect failed");
}

void WifiProvisionUi::Tick() {
    // reserved: spinner / retry backoff
}
