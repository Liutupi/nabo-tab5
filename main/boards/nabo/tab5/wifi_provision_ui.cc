#include "wifi_provision_ui.h"

#include <esp_log.h>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <utility>

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#define TAG "WifiProvUI"

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

    root_ = lv_obj_create(parent);
    lv_obj_set_size(root_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(root_, 16, 0);

    status_label_ = lv_label_create(root_);
    lv_label_set_text(status_label_, StateText(state_));
    lv_obj_align(status_label_, LV_ALIGN_TOP_MID, 0, 0);

    scan_button_ = lv_button_create(root_);
    lv_obj_set_size(scan_button_, 160, 56);
    lv_obj_align(scan_button_, LV_ALIGN_TOP_LEFT, 0, 40);
    auto* lab = lv_label_create(scan_button_);
    lv_label_set_text(lab, "扫描网络");
    lv_obj_center(lab);
    lv_obj_add_event_cb(scan_button_, [](lv_event_t* e) {
        auto* self = static_cast<WifiProvisionUi*>(lv_event_get_user_data(e));
        if (self) self->OnScanClicked();
    }, LV_EVENT_CLICKED, this);

    list_ = lv_list_create(root_);
    lv_obj_set_size(list_, lv_pct(55), 420);
    lv_obj_align(list_, LV_ALIGN_TOP_LEFT, 0, 120);

    password_textarea_ = lv_textarea_create(root_);
    lv_obj_set_size(password_textarea_, lv_pct(40), 56);
    lv_obj_align(password_textarea_, LV_ALIGN_TOP_RIGHT, 0, 120);
    lv_textarea_set_one_line(password_textarea_, true);
    lv_textarea_set_password_mode(password_textarea_, true);
    lv_textarea_set_placeholder_text(password_textarea_, "Wi-Fi 密码");
    lv_obj_add_flag(password_textarea_, LV_OBJ_FLAG_HIDDEN);

    keyboard_ = lv_keyboard_create(root_);
    lv_obj_set_size(keyboard_, lv_pct(40), 260);
    lv_obj_align(keyboard_, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_keyboard_set_textarea(keyboard_, password_textarea_);
    lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);

    connect_button_ = lv_button_create(root_);
    lv_obj_set_size(connect_button_, 160, 56);
    lv_obj_align(connect_button_, LV_ALIGN_TOP_RIGHT, 0, 200);
    auto* cl = lv_label_create(connect_button_);
    lv_label_set_text(cl, "连接");
    lv_obj_center(cl);
    lv_obj_add_flag(connect_button_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(connect_button_, [](lv_event_t* e) {
        auto* self = static_cast<WifiProvisionUi*>(lv_event_get_user_data(e));
        if (!self || !self->password_textarea_) return;
        self->OnPskConfirmed(lv_textarea_get_text(self->password_textarea_));
    }, LV_EVENT_CLICKED, this);

    spinner_ = lv_spinner_create(root_);
    lv_obj_set_size(spinner_, 48, 48);
    lv_obj_align(spinner_, LV_ALIGN_TOP_RIGHT, 0, 44);
    lv_obj_add_flag(spinner_, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
}

void WifiProvisionUi::Show() {
    if (root_) lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
    SetState(NABO_WIFI_IDLE);
}

void WifiProvisionUi::Hide() {
    if (root_) lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
    memset(psk_, 0, sizeof(psk_));
    if (password_textarea_) lv_textarea_set_text(password_textarea_, "");
}

void WifiProvisionUi::SetScanner(ScanFn scan) { scan_ = std::move(scan); }
void WifiProvisionUi::SetConnector(ConnectFn connect) { connect_ = std::move(connect); }
void WifiProvisionUi::SetStatusSink(StatusFn status) { status_ = std::move(status); }

void WifiProvisionUi::SetState(nabo_wifi_ui_state_t s) {
    state_ = s;
    if (status_label_) lv_label_set_text(status_label_, StateText(s));
    const bool show_psk = (s == NABO_WIFI_INPUT_PSK);
    if (password_textarea_) {
        if (show_psk) lv_obj_clear_flag(password_textarea_, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(password_textarea_, LV_OBJ_FLAG_HIDDEN);
    }
    if (keyboard_) {
        if (show_psk) lv_obj_clear_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
    }
    if (connect_button_) {
        if (show_psk) lv_obj_clear_flag(connect_button_, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(connect_button_, LV_OBJ_FLAG_HIDDEN);
    }
    if (spinner_) {
        if (s == NABO_WIFI_SCANNING || s == NABO_WIFI_CONNECTING) {
            lv_obj_clear_flag(spinner_, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(spinner_, LV_OBJ_FLAG_HIDDEN);
        }
    }
    ESP_LOGI(TAG, "state=%d", static_cast<int>(s));
}

void WifiProvisionUi::OnScanClicked() {
    SetState(NABO_WIFI_SCANNING);
    ap_count_ = 0;
    selected_ = -1;
    if (list_) lv_obj_clean(list_);
    if (scan_) ap_count_ = std::clamp(scan_(aps_, 20), 0, 20);
    if (list_) {
        for (int i = 0; i < ap_count_; ++i) {
            char line[80];
            snprintf(line, sizeof(line), "%s  %ddBm%s%s", aps_[i].ssid, aps_[i].rssi,
                     aps_[i].locked ? "  [加密]" : "", aps_[i].is_11ax ? "  Wi-Fi6" : "");
            auto* btn = lv_list_add_button(list_, nullptr, line);
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
    if (!connect_) {
        SetConnectionResult(false, "network service unavailable");
        return;
    }
    connect_(ssid_, psk_);
}

void WifiProvisionUi::SetConnectionResult(bool connected, const char* detail) {
    SetState(connected ? NABO_WIFI_SUCCESS : NABO_WIFI_ERROR);
    if (status_label_ && detail && detail[0]) lv_label_set_text(status_label_, detail);
    if (status_) status_(connected ? ssid_ : (detail ? detail : "connect failed"));
    memset(psk_, 0, sizeof(psk_));
    if (password_textarea_) lv_textarea_set_text(password_textarea_, "");
}

void WifiProvisionUi::Tick() {
    // reserved: spinner / retry backoff
}
