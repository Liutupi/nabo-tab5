#include "nabo_face.h"
#include "nabo_assets_gen.h"
#include "nabo_config.h"

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

static bool ClipNameToEmotion(const char* name, NaboEmotion* emotion) {
    if (!name || !emotion) return false;
    struct Mapping {
        const char* clip;
        NaboEmotion emotion;
    };
    static const Mapping kMappings[] = {
        {"idle", NaboEmotion::kIdle},           {"listen", NaboEmotion::kListening},
        {"listening", NaboEmotion::kListening}, {"think", NaboEmotion::kThinking},
        {"thinking", NaboEmotion::kThinking},   {"speak", NaboEmotion::kSpeaking},
        {"speaking", NaboEmotion::kSpeaking},   {"happy", NaboEmotion::kHappy},
        {"wave", NaboEmotion::kWave},           {"wake", NaboEmotion::kWake},
        {"tired", NaboEmotion::kTired},         {"sleep", NaboEmotion::kSleep},
        {"sad", NaboEmotion::kSad},             {"angry", NaboEmotion::kAngry},
    };
    for (const auto& mapping : kMappings) {
        if (strcasecmp(mapping.clip, name) == 0) {
            *emotion = mapping.emotion;
            return true;
        }
    }
    return false;
}

static std::string ResolveAssetPath(const char* frame) {
    if (!frame || !frame[0]) return {};
    constexpr const char* kGeneratedPrefix = "/nabo/";
    if (strncmp(frame, kGeneratedPrefix, strlen(kGeneratedPrefix)) == 0) {
        return std::string(NABO_ASSET_ROOT) + "/" + (frame + strlen(kGeneratedPrefix));
    }
    if (frame[0] == '/') return frame;
    return std::string(NABO_ASSET_ROOT) + "/" + frame;
}

const char* NaboFace::Name(NaboEmotion e) {
    const auto i = static_cast<size_t>(e);
    return i < static_cast<size_t>(NaboEmotion::kCount) ? kEmotionNames[i] : "idle";
}

void NaboFace::Create(void* parent) {
    auto* p = static_cast<lv_obj_t*>(parent);
    if (p) {
        img_ = lv_image_create(p);
        lv_obj_set_size(static_cast<lv_obj_t*>(img_), 512, 512);
        lv_obj_center(static_cast<lv_obj_t*>(img_));
    }
    // Resolve by stable clip name. The generated table may contain auxiliary
    // clips such as "blink", so manifest order must never drive enum indexes.
    for (size_t i = 0; i < kNaboClipCount; ++i) {
        NaboEmotion emotion;
        if (!ClipNameToEmotion(kNaboClips[i].name, &emotion)) {
            ESP_LOGI(TAG, "auxiliary clip retained outside primary state map: %s",
                     kNaboClips[i].name ? kNaboClips[i].name : "");
            continue;
        }
        auto& clip = clips_[static_cast<size_t>(emotion)];
        clip.name = kNaboClips[i].name;
        clip.fps = kNaboClips[i].fps > 0 ? kNaboClips[i].fps : 2;
        clip.loop = kNaboClips[i].loop != 0;
        clip.frames.clear();
        for (size_t f = 0; f < kNaboClips[i].frame_count; ++f) {
            if (kNaboClips[i].frames[f] && kNaboClips[i].frames[f][0]) {
                clip.frames.emplace_back(ResolveAssetPath(kNaboClips[i].frames[f]));
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
    if (e >= NaboEmotion::kCount) e = NaboEmotion::kIdle;
    if (initialized_ && e == current_ && !one_shot_) return;
    current_ = e;
    initialized_ = true;
    one_shot_ = false;
    frame_index_ = 0;
    last_frame_ms_ = esp_timer_get_time() / 1000;
    ShowFrame(0);
}

void NaboFace::SetEmotionByName(const char* name) {
    NaboEmotion emotion;
    if (ClipNameToEmotion(name, &emotion)) SetEmotion(emotion);
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
    if (clip.frames.empty()) return;
    const int64_t now = esp_timer_get_time() / 1000;
    const int64_t interval = 1000 / (clip.fps > 0 ? clip.fps : 2);
    if (clip.frames.size() == 1) {
        if (one_shot_ && now - last_frame_ms_ >= interval) {
            one_shot_ = false;
            SetEmotion(pending_);
        }
        return;
    }
    if (!clip.loop && !one_shot_ && frame_index_ + 1 >= clip.frames.size()) return;
    if (now - last_frame_ms_ >= interval) {
        last_frame_ms_ += interval;
        AdvanceAnim();
    }
}
