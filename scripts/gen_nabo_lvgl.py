#!/usr/bin/env python3
"""Generate LVGL-friendly asset index / optional C headers from nabo_assets."""
from __future__ import annotations

import json
from pathlib import Path

ASSETS = Path(r"D:\开发板SD卡\nabo")
OUT_H = Path(r"D:\tab5\nabo-tab5\main\boards\nabo-tab5\nabo_assets_gen.h")


def emit_header(manifest: dict) -> None:
    states = manifest.get("states", {})
    lines = [
        "#pragma once",
        "/* Auto-generated from nabo/manifest.json — do not edit */",
        "#include <stddef.h>",
        "",
        "typedef struct {",
        "  const char* name;",
        "  const char* const* frames;",
        "  size_t frame_count;",
        "  int fps;",
        "  int loop; /* 1/0 */",
        "} nabo_clip_t;",
        "",
    ]
    for state, meta in states.items():
        frames = meta.get("files", [])
        guard = state.replace("-", "_")
        lines.append(f"static const char* const kNaboFrames_{guard}[] = {{")
        folder = "bust" if state == "blink" else "full"
        for f in frames:
            lines.append(f'  "/nabo/{folder}/{f}",')
        if not frames:
            lines.append('  "",')
        lines.append("};")
        lines.append("")

    lines.append("static const nabo_clip_t kNaboClips[] = {")
    for state, meta in states.items():
        guard = state.replace("-", "_")
        frames = meta.get("files", [""])
        fps = int(meta.get("fps", 2))
        loop = 1 if meta.get("loop", False) else 0
        lines.append(
            f'  {{ "{state}", kNaboFrames_{guard}, {len(frames)}, {fps}, {loop} }},'
        )
    lines.append("};")
    lines.append("static const size_t kNaboClipCount = sizeof(kNaboClips)/sizeof(kNaboClips[0]);")
    lines.append("")
    OUT_H.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("WROTE", OUT_H)


def main() -> None:
    mf = ASSETS / "manifest.json"
    if not mf.exists():
        raise SystemExit(f"missing {mf}")
    emit_header(json.loads(mf.read_text(encoding="utf-8")))


if __name__ == "__main__":
    main()
