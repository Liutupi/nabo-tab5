#!/usr/bin/env python3
"""Generate the NABO LVGL clip index from a versioned manifest."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = REPO_ROOT / "main/boards/nabo/tab5/nabo_assets_gen.h"


def c_identifier(value: str) -> str:
    identifier = re.sub(r"[^A-Za-z0-9_]", "_", value.replace("-", "_"))
    if not identifier or identifier[0].isdigit():
        identifier = f"clip_{identifier}"
    return identifier


def c_string(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def emit_header(manifest: dict, output: Path, path_prefix: str) -> None:
    states = manifest.get("states")
    if not isinstance(states, dict) or not states:
        raise ValueError("manifest must contain a non-empty 'states' object")

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

    seen_identifiers: set[str] = set()
    normalized: list[tuple[str, str, list[str], int, int]] = []
    for state, meta in states.items():
        if not isinstance(state, str) or not isinstance(meta, dict):
            raise ValueError("state names must be strings and state metadata must be objects")
        identifier = c_identifier(state)
        if identifier in seen_identifiers:
            raise ValueError(f"duplicate C identifier generated for state {state!r}")
        seen_identifiers.add(identifier)

        frames = meta.get("files", [])
        if not isinstance(frames, list) or not all(isinstance(item, str) for item in frames):
            raise ValueError(f"state {state!r} must contain a string 'files' array")
        fps = int(meta.get("fps", 2))
        if fps <= 0 or fps > 60:
            raise ValueError(f"state {state!r} fps must be between 1 and 60")
        loop = 1 if bool(meta.get("loop", False)) else 0
        folder = "bust" if state == "blink" else "full"
        normalized.append((state, identifier, frames, fps, loop))

        lines.append(f"static const char* const kNaboFrames_{identifier}[] = {{")
        if frames:
            for filename in frames:
                if Path(filename).name != filename:
                    raise ValueError(f"state {state!r} contains an unsafe filename: {filename!r}")
                lines.append(f"  {c_string(f'{path_prefix}/{folder}/{filename}')},")
        else:
            lines.append('  "",')
        lines.extend(["};", ""])

    lines.append("static const nabo_clip_t kNaboClips[] = {")
    for state, identifier, frames, fps, loop in normalized:
        lines.append(
            f"  {{ {c_string(state)}, kNaboFrames_{identifier}, {len(frames)}, {fps}, {loop} }},"
        )
    lines.extend(
        [
            "};",
            "static const size_t kNaboClipCount = sizeof(kNaboClips)/sizeof(kNaboClips[0]);",
            "",
        ]
    )

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines), encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path, help="path to nabo manifest.json")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="generated header path")
    parser.add_argument("--path-prefix", default="/nabo", help="runtime path prefix stored in the header")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    manifest_path = args.manifest.resolve()
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    emit_header(manifest, args.output.resolve(), args.path_prefix.rstrip("/"))
    print(f"WROTE {args.output.resolve()}")


if __name__ == "__main__":
    main()
