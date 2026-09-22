# NABO-Tab5 推进状态

## 编译里程碑（2026-09-22）

- **官方 `m5stack-tab5` 在 ESP-IDF 6.0.2 下编译成功**
- 产物：`D:\tab5\xiaozhi-esp32\build\xiaozhi.bin`（0x37ea10，app 分区余 11%）
- 合并镜像：`build\merged-binary.bin`
- 源码树：`D:\tab5\xiaozhi-esp32`（含 `esp_lcd_st7121` / `st7123` 驱动，自动识别面板）
- 本地补丁：`managed_components/espressif__esp_lvgl_port/.../esp_lvgl_port_disp.c`
  DSI 回调改用 `on_refresh_done`（IDF 6 DSI 无 `on_frame_buf_complete`）
- 激活环境：`C:\Espressif\esp-idf-v6.0.2\export.bat`
- 编译命令：`python scripts\build.py m5stack/tab5 --name m5stack-tab5`

## 已完成

| 项 | 位置 |
|---|---|
| SD 资源审计 | `D:\tab5\SD_AUDIT.md` |
| NABO 资源入 SD 布局 | `D:\开发板SD卡\nabo\`（37 PNG + manifest） |
| macOS 垃圾清理 | calendar 下 14 个 `._*` / `.DS_Store` |
| ST7121 板级骨架 | `D:\tab5\nabo-tab5\` |
| 设置页配网状态机 + LVGL UI | `wifi_provision_ui.*` |
| NaboFace 状态机 + 资源表 | `nabo_face.*` + `nabo_assets_gen.h` |
| 桌面壳（Main/Xiaozhi/Settings/WiFi） | `desktop_ui.*` |
| 分区草图 / Kconfig | `partitions/nabo_tab5.csv`、`sdkconfig.nabo-tab5.defaults` |

## 配网状态机

`Idle → Scanning → List → InputPsk → Connecting → Success | Error`

对接：`WifiStation` 扫描/连接/NVS（接入 xiaozhi 树时填 lambda）。

## 下一步（需完整 xiaozhi 源码树 + ESP-IDF）

1. clone `78/xiaozhi-esp32` 或你的 fork → 并入 `main/boards/nabo-tab5/`
2. 从 M5GFX / ST7121 路径移植显示触摸驱动（注意 LCD_RST 经 PI4IOE）
3. 实现 `Tab5AudioCodec`（ES8388+ES7210）
4. 将 `DesktopUi::SetXiaozhiState` 接到 `Application`
5. `idf.py set-target esp32p4` → `build` → 烧录

## SD 待补

- `PHOTOS/` 示例 1280×720 JPG
- podcast：磁盘 mp3 与 `index.json`（80）数量不一致，需核对
- 说明文档改为 Tab5/ST7121 版
