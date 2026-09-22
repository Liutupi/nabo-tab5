# NABO-Tab5（ST7121）板级骨架

目标：M5Stack Tab5（**ST7121** 触控一体屏）+ 小智 AI 内核 + NABO 桌面 + **设置页 Wi-Fi 配网**。

## 硬件基线

- ESP32-P4 + ESP32-C6（Wi-Fi 6）
- 1280×720 MIPI-DSI **ST7121**
- ES8388 + ES7210 双麦 AEC
- 16MB Flash / 32MB PSRAM
- 详见 `../ARCHITECTURE_TAB5.md` 与 `../SD_AUDIT.md`

## 目录

```text
main/boards/nabo-tab5/
  config.h              # 引脚与面板批次
  config.json           # target esp32p4 / SKU nabo-tab5
  nabo_tab5_board.*     # Board 组装（WifiBoard）
  wifi_provision_ui.*   # 设置页独立配网
  nabo_face.*           # NABO 表情（读 /sdcard/nabo/manifest.json）
```

## SD 资源

见 `D:\开发板SD卡`（已含 `/nabo/`）。拷贝到 FAT32 卡根目录即可。

## 下一步（接入官方 xiaozhi 树）

1. 将本目录并入 `78/xiaozhi-esp32` 或你的 fork 的 `main/boards/nabo-tab5/`
2. `sdkconfig.defaults` 增加 `CONFIG_BOARD_TYPE_NABO_TAB5=y`
3. 从 `m5stack-tab5` / ST7121 M5GFX 路径移植显示触摸驱动
4. 实现 `wifi_provision_ui` LVGL 界面，对接 `WifiStation`
5. `nabo_face` 读 manifest 并挂到 Xiaozhi 页
6. `idf.py set-target esp32p4 && idf.py build`

## 状态机（配网）

`Idle → Scanning → List → InputPsk → Connecting → Success | Error`
