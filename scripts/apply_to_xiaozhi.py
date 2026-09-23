#!/usr/bin/env python3
"""Install the pinned NABO Tab5 board overlay into a XiaoZhi checkout."""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import tempfile
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


def install_esp_ipa_component(checkout: Path, lock: dict, force: bool) -> None:
    """Use the upstream Rev < 3 ISP library and a hardware-safe SC202CS profile."""
    component = lock["component_overrides"]["espressif/esp_ipa"]
    target = checkout / "components/espressif__esp_ipa"
    profile_target = checkout / "main/boards/nabo/tab5/sc202cs_ipa.json"
    if target.exists() and not force:
        raise SystemExit(f"target already exists: {target}; pass --force to replace it")

    with tempfile.TemporaryDirectory(prefix="nabo-esp-ipa-") as temp_dir:
        source = Path(temp_dir) / "esp-video-components"
        subprocess.run(
            ["git", "clone", "--filter=blob:none", "--sparse", "--no-checkout",
             component["repository"], str(source)],
            check=True,
        )
        subprocess.run(["git", "sparse-checkout", "set", component["path"],
                        "esp_cam_sensor/sensors/sc202cs/cfg"],
                       cwd=source, check=True)
        subprocess.run(["git", "checkout", "--detach", component["commit"]],
                       cwd=source, check=True)
        component_source = source / component["path"]
        expected_library = component_source / "lib/esp32p4/v6.0+/libesp_ipa_p4_less_v3.a"
        if not expected_library.is_file():
            raise RuntimeError(f"pinned ESP-IPA checkout lacks {expected_library}")
        if target.exists():
            shutil.rmtree(target)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(component_source, target)
        profile_source = source / "esp_cam_sensor/sensors/sc202cs/cfg/sc202cs_default.json"
        profile = json.loads(profile_source.read_text(encoding="utf-8"))
        ccm_table = profile["SC202CS"]["acc"]["ccm"]["table"]
        low_temp_matrix = next(item["matrix"] for item in ccm_table if item["color_temp"] == 2292)
        if low_temp_matrix[6:9] != [-0.7768, -2.7677, 4.5445]:
            raise RuntimeError("pinned SC202CS CCM profile changed; review correction before building")
        # Keep the blue-output row summing to 1 while bringing its peak below
        # the ESP32-P4 CCM hardware limit. This avoids repeated rejected writes.
        peak = 3.9
        scale = (peak - 1.0) / (low_temp_matrix[8] - 1.0)
        low_temp_matrix[6] *= scale
        low_temp_matrix[7] *= scale
        low_temp_matrix[8] = peak
        profile_target.write_text(json.dumps(profile, ensure_ascii=False, separators=(",", ":")) + "\n",
                                  encoding="utf-8")
    print(f"Installed ESP-IPA Rev < 3 fix {component['commit']} into {target}")
    print(f"Installed SC202CS CCM profile into {profile_target}")


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

    for relative in (
        "main/Kconfig.projbuild",
        "main/CMakeLists.txt",
        "scripts/build.py",
    ):
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
    install_esp_ipa_component(checkout, lock, args.force)

    print(f"Installed NABO Tab5 overlay into {checkout}")
    print("Build Rev < 3: python scripts/build.py nabo/tab5 --name nabo-tab5")
    print("Build Rev >= 3: python scripts/build.py nabo/tab5 --name nabo-tab5-p4x")


if __name__ == "__main__":
    main()
