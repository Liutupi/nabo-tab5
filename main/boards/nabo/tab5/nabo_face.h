#pragma once

#include <cstdint>
#include <string>
#include <vector>

/** NABO face clips driven by XiaoZhi device/emotion state. */
enum class NaboEmotion : uint8_t {
    kIdle = 0,
    kListening,
    kThinking,
    kSpeaking,
    kHappy,
    kWave,
    kWake,
    kTired,
    kSleep,
    kSad,
    kAngry,
    kCount,
};

class NaboFace {
 public:
    /** parent: lv_obj_t* */
    void Create(void* parent);
    void SetEmotion(NaboEmotion e);
    void SetEmotionByName(const char* name);
    void OnDeviceState(const char* state, const char* message);
    /** Play one-shot clip then fall back to `then` (wave/wake). */
    void PlayOnce(NaboEmotion clip, NaboEmotion then);
    void Tick();

    static const char* Name(NaboEmotion e);

 private:
    struct Clip {
        std::string name;
        std::vector<std::string> frames;  // paths under NABO_ASSET_ROOT
        int fps = 2;
        bool loop = true;
    };

    bool LoadManifest(const char* root);
    void ShowFrame(size_t index);
    void AdvanceAnim();

    NaboEmotion current_ = NaboEmotion::kIdle;
    NaboEmotion pending_ = NaboEmotion::kIdle;
    bool initialized_ = false;
    bool one_shot_ = false;
    Clip clips_[static_cast<size_t>(NaboEmotion::kCount)];
    size_t frame_index_ = 0;
    int64_t last_frame_ms_ = 0;
    void* img_ = nullptr;
};
