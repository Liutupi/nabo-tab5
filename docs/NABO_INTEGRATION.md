# NABO XiaoZhi Animation Module — Tab5 Integration Sketch

Assets root: `D:\tab5\nabo_assets\`  
Manifest: `nabo_assets/manifest.json`

## Layout

```
nabo_assets/
  manifest.json
  reference_sheet.png
  full/          # 512x512, bottom-center aligned full body
    idle_01..05.png
    listen_01..02.png
    think_01..02.png
    speak_01..06.png
    happy_01..02.png
    wave_01..04.png      # sequence
    wave_pose_01..03.png # static poses
    wake_01..04.png
    tired_01.png
    sleep_01.png
    sad_01.png
    angry_01.png
  bust/          # 384x384, center aligned chest-up
    blink_01..04.png
```

## Device state → clip

| DeviceState / emotion | Clip | Play mode | fps |
|---|---|---|---|
| idle | `idle` + random `blink` | loop + overlay | 2 / 10 |
| listening | `listen` | hold | 1 |
| thinking | `think` | hold | 1 |
| speaking | `speak` | loop | 8 |
| happy | `happy` | loop | 2 |
| greeting / boot | `wave` then idle | once | 6 |
| wake | `wake` then idle | once | 4 |
| tired / error | `tired` | hold | 1 |
| sleep | `sleep` | hold | 1 |
| sad | `sad` | hold | 1 |
| angry | `angry` | hold | 1 |

## C++ API sketch (board-local)

```cpp
// nabo_face.h
#pragma once
#include <cstdint>

enum class NaboEmotion : uint8_t {
  kIdle, kListening, kThinking, kSpeaking,
  kHappy, kWave, kWake, kTired, kSleep, kSad, kAngry,
};

class NaboFace {
 public:
  void Create(lv_obj_t* parent);
  void SetEmotion(NaboEmotion e);
  void SetEmotionByName(const char* name);  // "idle" | "listening" | ...
  void OnDeviceState(const char* state, const char* message);
  void PlayOnce(NaboEmotion e, NaboEmotion then);  // wave/wake then idle
  void Tick();  // drive blink overlay timer
 private:
  NaboEmotion current_ = NaboEmotion::kIdle;
  lv_obj_t* img_ = nullptr;
  lv_timer_t* blink_timer_ = nullptr;
  // frame tables generated from manifest.json → nabo_assets_gen.h
};
```

## Integration points (xiaozhi / QDTech style)

```cpp
// DesktopUI::SetXiaozhiState / SetXiaozhiEmotion
void DesktopUI::SetXiaozhiEmotion(const char* emotion) {
  if (nabo_face_) nabo_face_->SetEmotionByName(emotion);
}

void DesktopUI::SetXiaozhiState(const char* state, const char* message, const char* emotion) {
  // map application state → NaboEmotion
  if (strcmp(state, "listening") == 0) nabo_face_->SetEmotion(NaboEmotion::kListening);
  else if (strcmp(state, "speaking") == 0) nabo_face_->SetEmotion(NaboEmotion::kSpeaking);
  else if (strcmp(state, "thinking") == 0) nabo_face_->SetEmotion(NaboEmotion::kThinking);
  else if (strcmp(state, "idle") == 0) nabo_face_->SetEmotion(NaboEmotion::kIdle);
  // ...
}
```

## Resource conversion (later)

1. `lv_img_conv` / custom script: PNG → `LV_IMG_CF_RGB565A8` or indexed + alpha.
2. Emit `nabo_assets_gen.h` with `const lv_img_dsc_t` arrays (Tab5 32MB PSRAM can also load from SD).
3. Keep `manifest.json` as the source of truth for frame order and fps.

## Notes

- `angry_01` is the user-accepted stand-in (reads visually as pout/upset). Swap later without code change.
- Full body canvas 512x512 bottom-aligned so wave/wake feet stay stable.
- Blink is bust-only overlay to save RAM; can upgrade to full-body blink later.
