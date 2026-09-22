#include "desktop_ui.h"
#include "wifi_provision_ui.h"

#include <esp_log.h>
#include <string.h>

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#define TAG "DesktopUI"

void DesktopUi::Create(int w, int h) {
    root_ = lv_screen_active();
    (void)w;
    (void)h;
    // Phase-1: Xiaozhi full-screen face + top status.
    // Main/Settings switch from a small tab bar (add later).
    ShowPage(Page::kXiaozhi);
}

void DesktopUi::ShowPage(Page p) {
    page_ = p;
    if (p == Page::kWifi && wifi_) wifi_->Show();
    if (p != Page::kWifi && wifi_) wifi_->Hide();
}

void DesktopUi::ShowMain() { ShowPage(Page::kMain); }
void DesktopUi::ShowXiaozhi() { ShowPage(Page::kXiaozhi); }
void DesktopUi::ShowSettings() { ShowPage(Page::kSettings); }
void DesktopUi::ShowWifiProvision() { ShowPage(Page::kWifi); }

void DesktopUi::SetXiaozhiState(const char* state, const char* message, const char* emotion) {
    ESP_LOGI(TAG, "state=%s emotion=%s msg=%s", state ? state : "", emotion ? emotion : "",
             message ? message : "");
    if (face_) face_->OnDeviceState(state, message);
    if (emotion && face_) face_->SetEmotionByName(emotion);
}

void DesktopUi::SetClock(const char* text) {
    if (label_clock_ && text) lv_label_set_text(static_cast<lv_obj_t*>(label_clock_), text);
}

void DesktopUi::SetNetworkStatus(const char* text) {
    if (label_net_ && text) lv_label_set_text(static_cast<lv_obj_t*>(label_net_), text);
}

void DesktopUi::Tick() {
    if (face_) face_->Tick();
    if (wifi_) wifi_->Tick();
}
