#include "nabo_face.h"
#include "nabo_assets_gen.h"
#include "config.h"

#include <esp_log.h>
#include <esp_timer.h>
#include <string.h>
#include <stdio.h>

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#define TAG "NaboFace"

static const char* kEmotionNames[] = {
    "idle", "listening", "thinking", "speaking", "happy",
    "wave", "wake", "tired", "sleep", "sad", "angry",
};

const char* NaboFace::Name(NaboEmotion e) {
    const auto i = static_cast<size_t>(e);
    return i < static_cast<size_t>(NaboEmotion::kCount) ? kEmotionNames[i] : "idle";
}

void NaboFace::Create(void* parent) {
    auto* p = static_cast<lv_obj_t*>(parent);
    if (p) {
        img_ = lv_image_create(p);
        lv_obj_set_size(static_cast<lv_obj_t*>(img_), 360, 360);
        lv_obj_center(static_cast<lv_obj_t*>(img_));
    }
    // Prefer generated tables (compiled); SD paths inside arrays.
    for (size_t i = 0; i < kNaboClipCount && i < static_cast<size_t>(NaboEmotion::kCount); ++i) {
        clips_[i].name = kNaboClips[i].name;
        clips_[i].fps = kNaboClips[i].fps > 0 ? kNaboClips[i].fps : 2;
        clips_[i].loop = kNaboClips[i].loop != 0;
        clips_[i].frames.clear();
        for (size_t f = 0; f < kNaboClips[i].frame_count; ++f) {
            if (kNaboClips[i].frames[f] && kNaboClips[i].frames[f][0]) {
                clips_[i].frames.emplace_back(kNaboClips[i].frames[f]);
            }
        }
    }
    LoadManifest(NABO_ASSET_ROOT);  // optional runtime override
    ESP_LOGI(TAG, "ready clips=%u root=%s", (unsigned)kNaboClipCount, NABO_ASSET_ROOT);
    SetEmotion(NaboEmotion::kIdle);
}

bool NaboFace::LoadManifest(const char* root) {
    // Runtime SD manifest can override compiled tables after OTA asset swap.
    char path[128];
    snprintf(path, sizeof(path), "%s/manifest.json", root);
    FILE* fp = fopen(path, "r");
    if (!fp) return false;
    fclose(fp);
    ESP_LOGW(TAG, "SD manifest present at %s (optional override not parsed yet)", path);
    return true;
}

void NaboFace::SetEmotion(NaboEmotion e) {
    if (e == current_ && !one_shot_) return;
    current_ = e;
    one_shot_ = false;
    frame_index_ = 0;
    last_frame_ms_ = esp_timer_get_time() / 1000;
    ShowFrame(0);
}

void NaboFace::SetEmotionByName(const char* name) {
    if (!name) return;
    for (size_t i = 0; i < static_cast<size_t>(NaboEmotion::kCount); ++i) {
        if (strcasecmp(kEmotionNames[i], name) == 0) {
            SetEmotion(static_cast<NaboEmotion>(i));
            return;
        }
    }
}

void NaboFace::OnDeviceState(const char* state, const char* message) {
    (void)message;
    if (!state) return;
    if (strcmp(state, "listening") == 0) SetEmotion(NaboEmotion::kListening);
    else if (strcmp(state, "speaking") == 0) SetEmotion(NaboEmotion::kSpeaking);
    else if (strcmp(state, "connecting") == 0) SetEmotion(NaboEmotion::kThinking);
    else if (strcmp(state, "thinking") == 0) SetEmotion(NaboEmotion::kThinking);
    else if (strcmp(state, "idle") == 0) SetEmotion(NaboEmotion::kIdle);
    else if (strcmp(state, "upgrading") == 0) SetEmotion(NaboEmotion::kThinking);
    else if (strcmp(state, "error") == 0) SetEmotion(NaboEmotion::kSad);
    else SetEmotionByName(state);
}

void NaboFace::PlayOnce(NaboEmotion clip, NaboEmotion then) {
    current_ = clip;
    pending_ = then;
    one_shot_ = true;
    frame_index_ = 0;
    last_frame_ms_ = esp_timer_get_time() / 1000;
    ShowFrame(0);
}

void NaboFace::ShowFrame(size_t index) {
    const auto& clip = clips_[static_cast<size_t>(current_)];
    if (!img_ || index >= clip.frames.size()) return;
    lv_image_set_src(static_cast<lv_obj_t*>(img_), clip.frames[index].c_str());
}

void NaboFace::AdvanceAnim() {
    const auto& clip = clips_[static_cast<size_t>(current_)];
    if (clip.frames.size() <= 1) {
        if (one_shot_) {
            one_shot_ = false;
            SetEmotion(pending_);
        }
        return;
    }
    frame_index_++;
    if (frame_index_ >= clip.frames.size()) {
        if (one_shot_) {
            one_shot_ = false;
            SetEmotion(pending_);
            return;
        }
        frame_index_ = clip.loop ? 0 : clip.frames.size() - 1;
    }
    ShowFrame(frame_index_);
}

void NaboFace::Tick() {
    const auto& clip = clips_[static_cast<size_t>(current_)];
    if (clip.frames.size() <= 1) return;
    const int64_t now = esp_timer_get_time() / 1000;
    const int64_t interval = 1000 / (clip.fps > 0 ? clip.fps : 2);
    if (now - last_frame_ms_ >= interval) {
        last_frame_ms_ = now;
        AdvanceAnim();
    }
}
