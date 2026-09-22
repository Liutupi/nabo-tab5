# HANDOFF — NABO-Tab5 / 小智（交接本）

> 日期：2026-09-22  
> 目标：M5Stack **Tab5（ST7121）** 上的 NABO 桌面 + 小智 AI + 设置页 Wi-Fi 配网  
> 开源仓：https://github.com/Liutupi/nabo-tab5  
> 本地项目根：`D:\tab5`（MiMo 项目名 `tan5`）

---

## 1. 一句话现状

**官方小智 `m5stack-tab5` 已在 ESP-IDF 6.0.2 编译通过**；NABO 资产已整理；板级骨架 / 配网 UI / NaboFace 接口已写好，**尚未并进 xiaozhi 树的完整构建**。

---

## 2. 关键路径

| 用途 | 路径 |
|---|---|
| 开源仓工作副本 | `D:\tab5\nabo-tab5\` |
| GitHub | https://github.com/Liutupi/nabo-tab5 |
| 官方小智源码树（本地，勿当开源仓） | `D:\tab5\xiaozhi-esp32\` |
| 已编译固件 | `D:\tab5\xiaozhi-esp32\build\xiaozhi.bin`（3.49MB） |
| 合并烧录镜像 | `D:\tab5\xiaozhi-esp32\build\merged-binary.bin`（10.85MB） |
| NABO 资源（整理后） | `D:\tab5\nabo_assets\` |
| SD 资源包（QDTech 风格） | `D:\开发板SD卡\`（含新建 `/nabo/`） |
| SD 审计 | `D:\tab5\SD_AUDIT.md` |
| 架构设计 | `D:\tab5\ARCHITECTURE_TAB5.md` |
| ESP-IDF 6.0.2 | `C:\Espressif\esp-idf-v6.0.2\` |
| 激活环境 | `C:\Espressif\esp-idf-v6.0.2\export.bat` |

---

## 3. 构建命令（可复现）

```bat
cd /d D:\tab5\xiaozhi-esp32
call C:\Espressif\esp-idf-v6.0.2\export.bat
python scripts\build.py m5stack/tab5 --name m5stack-tab5
```

烧录（设备进下载模式后）：

```bat
idf.py -p COMx flash
:: 或
idf.py -p COMx flash monitor
```

下载模式：长按 Reset 约 2 秒，绿灯快闪后松开。

---

## 4. 硬件与驱动

- Tab5：ESP32-P4 + ESP32-C6 Wi-Fi 6，16MB Flash，32MB PSRAM，5" 1280×720 MIPI-DSI
- 用户设备屏幕驱动：**ST7121**（2025-10 后 TDDI 一体触控）
- 官方 `main/boards/m5stack/tab5/` 已含 `esp_lcd_st7121.c` / `esp_lcd_st7123.c`，按触控固件版本自动选择
- LCD_RST 经 PI4IOE：低=推挽/开漏，高=输入上拉（文档要求，避免 I2C 不稳）
- 硬件资料：https://docs.m5stack.com/en/core/Tab5

---

## 5. 已完成清单

### 5.1 编译链
- [x] clone `78/xiaozhi-esp32` → `D:\tab5\xiaozhi-esp32`
- [x] ESP-IDF 6.0.2 + tools（esp32p4 / riscv32）
- [x] **`m5stack-tab5` 完整编译成功**，app 余 11%
- [x] 本地补丁：`esp_lvgl_port` DSI 回调 `on_frame_buf_complete` → `on_refresh_done`（IDF 6 DSI 字段名）

### 5.2 NABO 资产
- [x] 归档到 `nabo_assets/full|bust` + `manifest.json` + `preview_contact_sheet.png`
- [x] 序列：blink 4 帧、wave 4 帧、wake 4 帧
- [x] 静态：idle/listen/think/speak/happy/tired/sleep/sad/angry
- [x] SD 布局：`D:\开发板SD卡\nabo\`
- [x] `scripts/gen_nabo_lvgl.py` → `nabo_assets_gen.h`

### 5.3 产品骨架（`nabo-tab5` 仓内）
- [x] `config.h`（ST7121 / P4 引脚）
- [x] `wifi_provision_ui.*` 设置页配网状态机 + LVGL UI
- [x] `nabo_face.*` 表情状态机
- [x] `desktop_ui.*` 页面壳
- [x] `nabo_tab5_board.*` Board 组装草图
- [x] 分区 `partitions/nabo_tab5.csv`、Kconfig 默认值

### 5.4 文档
- [x] `ARCHITECTURE_TAB5.md` 分层架构（借鉴 QDTech）
- [x] `SD_AUDIT.md` SD 资源缺口与优化
- [x] `docs/NABO_INTEGRATION.md` 接口草图
- [x] 本 HANDOFF

---

## 6. 未完成 / 下一步（按优先级）

### P0 — 硬件验证
1. Tab5 进下载模式，烧录 `xiaozhi.bin` 或 `merged-binary.bin`
2. 确认 ST7121 显示、触摸、扬声器、麦克风、Wi-Fi C6
3. 记录串口 log：`SKU=m5stack-tab5`、面板探测、Hosted Wi-Fi

### P1 — 并入 NABO 板级
1. 将 `nabo-tab5/main/boards/nabo-tab5/` 合并进 `xiaozhi-esp32/main/boards/nabo/tab5/`（注意 build.py 要求 `manufacturer/board` 目录）
2. 注册 `CONFIG_BOARD_TYPE_NABO_TAB5` 到 `Kconfig.projbuild` + `main/CMakeLists.txt`
3. 以官方 `m5stack_tab5.cc` 为基座，挂 `NaboFace` + `WifiProvisionUi`
4. 用 `python scripts\build.py nabo/tab5 --name nabo-tab5` 出独立 SKU 固件

### P2 — 产品功能
1. 设置页 Wi-Fi：对接 `WifiStation` 扫描/连接/NVS（Wi-Fi 6 经 C6/esp_hosted）
2. NABO 表情：读 `/sdcard/nabo/manifest.json` 或嵌入 `nabo_assets_gen.h`
3. `DesktopUi::SetXiaozhiState` → Application 状态机
4. SD：`radio.json`、PHOTOS 示例、podcast 索引核对（80 vs 160 mp3）

### P3 — 可选
- 电台 / 照片 / 播客 / 益智（QDTech 服务层可搬）
- BMI270 Shake Lab（Tab5 同款 IMU）
- OTA 独立通道

---

## 7. 必须知道的坑

1. **`esp_lvgl_port` 与 IDF 6 DSI**：已在 managed_components 本地改过；`idf.py reconfigure` 若重拉组件可能被覆盖，需重打补丁或改用兼容版本。
2. **屏幕批次**：旧固件只适配 ILI9881C+GT911；ST7121/ST7123 需新板级（官方 tab5 已支持）。
3. **`build.py` 板型名**：用 `m5stack/tab5`（目录名），不是 `m5stack-tab5`；variant 用 `--name m5stack-tab5`。
4. **自定义板目录**：需放在 `main/boards/<manufacturer>/<board>/`，且 `config.json` 有 `"type"`、`"manufacturer"`。
5. **勿把** `managed_components/`、`build/`、`sdkconfig` 当源码提交到开源仓（`.gitignore` 已配）。
6. **SD 大文件**：3.7GB 播客等不要进 Git，只提交目录约定。
7. P4 无原生 Wi-Fi：配网失败时优先查 C6 / esp_hosted / SDIO。

---

## 8. NABO 资产状态映射（摘要）

| 设备状态 | 资源 | 帧数 |
|---|---|---|
| idle | full/idle_* | 5 |
| listening | full/listen_* | 2 |
| thinking | full/think_* | 2 |
| speaking | full/speak_* | 6（口型循环） |
| happy | full/happy_* | 2 |
| wave | full/wave_* + wave_pose_* | 4+3 |
| wake | full/wake_* | 4 |
| blink | bust/blink_* | 4（idle 叠加） |
| tired / sleep / sad / angry | 各 1 张 | 定格 |

`angry` 为用户确认接受的 pout 图（视觉偏难过）；可后期替换 `nabo_angry.png`。

---

## 9. Git 状态（开源仓）

- 仓库：`Liutupi/nabo-tab5`（public，MIT）
- 分支：`main`
- 本交接提交：HANDOFF + PROGRESS + 构建里程碑说明
- 作者身份：`Liutupi@users.noreply.github.com`（未改全局 git config）

**本地 `D:\tab5\xiaozhi-esp32` 与 `C:\Espressif\` 不在该仓库内**，需按第 3 节命令在目标机器重建。

---

## 10. 快速恢复清单（换机器）

1. 装 ESP-IDF 6.0.2（`install.bat esp32p4`）
2. clone `https://github.com/78/xiaozhi-esp32.git`（或保留本地树）
3. clone `https://github.com/Liutupi/nabo-tab5.git`
4. 把 `nabo-tab5` 板级合入 xiaozhi（见 6.P1）
5. 打 lvgl_port DSI 补丁（见 7.1）或等上游修复
6. `build.py m5stack/tab5 --name m5stack-tab5` 先通官方板
7. SD 卡按 `docs/sd-layout/` + `D:\开发板SD卡` 结构准备
