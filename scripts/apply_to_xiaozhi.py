#!/usr/bin/env python3
"""Install the pinned NABO Tab5 board overlay into a XiaoZhi checkout."""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
BOARD_SOURCE = REPO_ROOT / "main/boards/nabo/tab5"
UPSTREAM_LOCK = REPO_ROOT / "upstream.json"

KCONFIG_ANCHOR = '''    config BOARD_TYPE_M5STACK_CORE_TAB5
        bool "M5Stack Tab5"
        depends on IDF_TARGET_ESP32P4
'''
KCONFIG_NABO = '''    config BOARD_TYPE_NABO_TAB5
        bool "NABO Tab5"
        depends on IDF_TARGET_ESP32P4
'''

CMAKE_ANCHOR = '''elseif(CONFIG_BOARD_TYPE_M5STACK_CORE_TAB5)
    set(BOARD_DIR "m5stack/tab5")
    set(BUILTIN_TEXT_FONT font_noto_sans_basic_30_4)
    set(BUILTIN_ICON_FONT font_material_symbols_30_4)
    set(DEFAULT_EMOJI_COLLECTION noto-color-emoji_64)
'''
CMAKE_NABO = '''elseif(CONFIG_BOARD_TYPE_NABO_TAB5)
    set(BOARD_DIR "nabo/tab5")
    set(BUILTIN_TEXT_FONT font_noto_sans_basic_30_4)
    set(BUILTIN_ICON_FONT font_material_symbols_30_4)
    set(DEFAULT_EMOJI_COLLECTION noto-color-emoji_64)
'''


def insert_after(path: Path, anchor: str, addition: str) -> None:
    text = path.read_text(encoding="utf-8")
    if addition in text:
        return
    if anchor not in text:
        raise RuntimeError(f"upstream layout changed; anchor not found in {path}")
    path.write_text(text.replace(anchor, anchor + addition, 1), encoding="utf-8")


def git_head(checkout: Path) -> str:
    result = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=checkout,
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout.strip()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("checkout", type=Path, help="path to a xiaozhi-esp32 checkout")
    parser.add_argument("--force", action="store_true", help="replace an existing nabo/tab5 overlay")
    parser.add_argument(
        "--allow-unpinned",
        action="store_true",
        help="allow a XiaoZhi commit other than the one recorded in upstream.json",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    checkout = args.checkout.resolve()
    lock = json.loads(UPSTREAM_LOCK.read_text(encoding="utf-8"))

    for relative in ("main/Kconfig.projbuild", "main/CMakeLists.txt", "scripts/build.py"):
        if not (checkout / relative).is_file():
            raise SystemExit(f"not a XiaoZhi checkout: missing {relative}")

    current_head = git_head(checkout)
    if current_head != lock["commit"] and not args.allow_unpinned:
        raise SystemExit(
            f"XiaoZhi HEAD is {current_head}; expected {lock['commit']}. "
            "Checkout the pinned commit or pass --allow-unpinned after reviewing upstream changes."
        )

    board_target = checkout / "main/boards/nabo/tab5"
    if board_target.exists():
        if not args.force:
            raise SystemExit(f"target already exists: {board_target}; pass --force to replace it")
        shutil.rmtree(board_target)
    board_target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(BOARD_SOURCE, board_target)

    insert_after(checkout / "main/Kconfig.projbuild", KCONFIG_ANCHOR, KCONFIG_NABO)
    insert_after(checkout / "main/CMakeLists.txt", CMAKE_ANCHOR, CMAKE_NABO)

    print(f"Installed NABO Tab5 overlay into {checkout}")
    print("Build Rev < 3: python scripts/build.py nabo/tab5 --name nabo-tab5")
    print("Build Rev >= 3: python scripts/build.py nabo/tab5 --name nabo-tab5-p4x")


if __name__ == "__main__":
    main()
