#pragma once
/* Auto-generated from nabo/manifest.json — do not edit */
#include <stddef.h>

typedef struct {
  const char* name;
  const char* const* frames;
  size_t frame_count;
  int fps;
  int loop; /* 1/0 */
} nabo_clip_t;

static const char* const kNaboFrames_idle[] = {
  "/nabo/full/idle_01.png",
  "/nabo/full/idle_02.png",
  "/nabo/full/idle_03.png",
  "/nabo/full/idle_04.png",
  "/nabo/full/idle_05.png",
};

static const char* const kNaboFrames_listen[] = {
  "/nabo/full/listen_01.png",
  "/nabo/full/listen_02.png",
};

static const char* const kNaboFrames_think[] = {
  "/nabo/full/think_01.png",
  "/nabo/full/think_02.png",
};

static const char* const kNaboFrames_speak[] = {
  "/nabo/full/speak_01.png",
  "/nabo/full/speak_02.png",
  "/nabo/full/speak_03.png",
  "/nabo/full/speak_04.png",
  "/nabo/full/speak_05.png",
  "/nabo/full/speak_06.png",
};

static const char* const kNaboFrames_happy[] = {
  "/nabo/full/happy_01.png",
  "/nabo/full/happy_02.png",
};

static const char* const kNaboFrames_wave[] = {
  "/nabo/full/wave_pose_01.png",
  "/nabo/full/wave_pose_02.png",
  "/nabo/full/wave_pose_03.png",
  "/nabo/full/wave_01.png",
  "/nabo/full/wave_02.png",
  "/nabo/full/wave_03.png",
  "/nabo/full/wave_04.png",
};

static const char* const kNaboFrames_wake[] = {
  "/nabo/full/wake_01.png",
  "/nabo/full/wake_02.png",
  "/nabo/full/wake_03.png",
  "/nabo/full/wake_04.png",
};

static const char* const kNaboFrames_blink[] = {
  "/nabo/bust/blink_01.png",
  "/nabo/bust/blink_02.png",
  "/nabo/bust/blink_03.png",
  "/nabo/bust/blink_04.png",
};

static const char* const kNaboFrames_tired[] = {
  "/nabo/full/tired_01.png",
};

static const char* const kNaboFrames_sleep[] = {
  "/nabo/full/sleep_01.png",
};

static const char* const kNaboFrames_sad[] = {
  "/nabo/full/sad_01.png",
};

static const char* const kNaboFrames_angry[] = {
  "/nabo/full/angry_01.png",
};

static const nabo_clip_t kNaboClips[] = {
  { "idle", kNaboFrames_idle, 5, 2, 1 },
  { "listen", kNaboFrames_listen, 2, 1, 0 },
  { "think", kNaboFrames_think, 2, 1, 0 },
  { "speak", kNaboFrames_speak, 6, 8, 1 },
  { "happy", kNaboFrames_happy, 2, 2, 1 },
  { "wave", kNaboFrames_wave, 7, 6, 0 },
  { "wake", kNaboFrames_wake, 4, 4, 0 },
  { "blink", kNaboFrames_blink, 4, 10, 0 },
  { "tired", kNaboFrames_tired, 1, 1, 0 },
  { "sleep", kNaboFrames_sleep, 1, 1, 0 },
  { "sad", kNaboFrames_sad, 1, 1, 0 },
  { "angry", kNaboFrames_angry, 1, 1, 0 },
};
static const size_t kNaboClipCount = sizeof(kNaboClips)/sizeof(kNaboClips[0]);

