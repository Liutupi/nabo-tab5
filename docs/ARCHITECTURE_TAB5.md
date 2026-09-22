# Tab5 × 小智 AI 框架设计（借鉴 QDTech 开源架构）

> 目标：以官方/开源小智协议栈为核心，仿照 `qdtech-s3-touch-lcd-3.5-xiaozhi-firmware` 的分层，
> 在 **M5Stack Tab5** 上做可触控大屏产品固件；设置页提供 **独立 Wi-Fi 配网界面**（Wi-Fi 6 / C6）。

---

## 1. 硬件资料（已核实）

| 来源 | 链接 / 路径 |
|---|---|
| **产品文档（主）** | https://docs.m5stack.com/en/core/Tab5 |
| 产品文档（中文） | https://docs.m5stack.com/zh_CN/core/Tab5 |
| 原理图 PDF | https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1132/Tab5_Schematics_PDF.pdf |
| 总体框图 | https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1132/Tab5_Overall_Design_Block_Diagram.pdf |
| PinMap 总览 | https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1132/C145_Pinmap_Overview.png |
| ST7123 TDDI 协议 | https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1132/ST7123-TDDI-Interface-Protocol-V01.11.pdf.pdf |
| 官方 BSP | `espressif/m5stack_tab5`（Component Registry，目标 `esp32p4`） |
| 出厂 Demo | https://github.com/m5stack/M5Tab5-UserDemo |
| IDF 编译教程 | https://docs.m5stack.com/en/esp_idf/m5tab5/userdemo（推荐 IDF v5.4.2，xiaozhi 可用 5.5） |
| C6 Wi-Fi 恢复 | https://docs.m5stack.com/en/guide/restore_factory/m5tab5_c6_wifi |

### 与固件相关的关键硬件

| 子系统 | 规格 | 对小智的意义 |
|---|---|---|
| 主控 | ESP32-P4NRW32，RISC-V 双核 360 MHz | 无原生 Wi-Fi，算力/PSRAM 强 |
| Flash / PSRAM | 16 MB / 32 MB Octal | 比 QDTech S3 宽裕，可放 NABO 全套 |
| 无线 | **ESP32-C6-MINI-1U，Wi-Fi 6**，SDIO 连 P4 | 配网/联网走 C6 |
| 显示 | 5" IPS **1280×720** MIPI-DSI | 横屏桌面产品 UI |
| 触摸 | 早期 GT911；**2025-10 起 ST7123/ST7121 TDDI** | 必须按批次区分驱动 |
| 音频 | ES8388 + ES7210 双麦 AEC，1W 喇叭 | 语音全双工/回声消除 |
| IMU | BMI270（可中断唤醒） | Shake Lab / 摇一摇 |
| RTC | RX8130CE | 定时唤醒、离线时钟 |
| 电源 | IP2326 + INA226 + NP-F550 | 电量/充电 UI |
| IO 扩展 | PI4IOE5V6408 ×2（0x43/0x44） | SPK_EN、LCD_RST、USB5V、WLAN_PWR 等 |
| 存储 | microSD | 电台/照片/播客/ROM |
| 其它 | SC2356 相机、RS485、USB Host/OTG | 后续扩展 |

**屏幕驱动批次（必做）：** 背部贴纸区分 `ILI9881C+GT911` 与 `ST7123/ST7121`。BSP 1.x 与 M5Unified 已兼容新驱动；固件需在板级做探测/双路径。

---

## 2. 架构原则（从 QDTech 项目继承）

1. **小智协议与语音是产品内核**，不与 UI/媒体写死在同一文件。  
2. **硬件绑定只在 board 目录**，服务层（天气/电台/配网/AudioFocus）可复用。  
3. **桌面以小智为中心**：Xiaozhi 页是主交互；其它页是应用磁贴。  
4. **配网是设置内一等公民**，大屏直接扫网/输密/连接，SoftAP 作降级。  
5. **实验功能用 Kconfig 开关**，默认关；稳定路径可回退。  
6. **OTA 板型/SKU 唯一**（`m5stack-tab5` / `nabo-tab5`），避免被通用包覆盖。

---

## 3. 分层架构

```text
┌─────────────────────────────────────────────────────────┐
│  Desktop UI（720×320→720p 重排 / NABO 主题）              │
│  Main │ Apps │ Xiaozhi │ Media │ Settings*Wifi* │ …      │
├─────────────────────────────────────────────────────────┤
│  服务层 Services                                           │
│  TimeWeather │ Radio │ Podcast │ AudioFocus │ WifiProvision │
│  NaboFace │ Profile/NVS │ OTA │ Power(INA226)              │
├─────────────────────────────────────────────────────────┤
│  小智内核 Core（尽量只用官方 API）                           │
│  Application 状态机 │ WebSocket/MQTT │ ESP-SR AFE/WakeNet   │
│  设备端 MCP ThingManager │ OPUS 音频流                       │
├─────────────────────────────────────────────────────────┤
│  Board 板级（main/boards/m5stack-tab5 或 nabo-tab5）        │
│  M5StackTab5Board : WifiBoard                              │
│  Tab5AudioCodec │ MipiLcdDisplay │ Touch(GT911/ST712x)     │
│  Pi4ioe │ Backlight │ Battery/RTC ─────────────────────────┼──┐
├─────────────────────────────────────────────────────────┤  │
│  无线与系统 HAL                                              │  │
│  ESP32-C6 (Wi-Fi 6) via SDIO / esp_hosted 或 M5 路径         │  │
│  MIPI-DSI │ I2S │ I2C │ SDMMC │ BMI270                      │  │
└─────────────────────────────────────────────────────────┘  │
         Tab5 硬件 ◄──────────────────────────────────────────┘
```

与 QDTech 对照：

| QDTech | Tab5 对应 | 策略 |
|---|---|---|
| ESP32-S3 WifiBoard | ESP32-P4 + C6 WifiBoard | 保留 WifiBoard 抽象，底层换 hosted/C6 |
| ST77922 480×320 横屏 | MIPI-DSI 1280×720 | DesktopUI 布局重做，页面模型沿用 |
| CST9217 手写触控 | GT911 / ST712x | 建议直接 `lv_indev`，少踩 QDTech 手写命中坑 |
| ES8311 | ES8388+ES7210 | 已有 `tab5_audio_codec` |
| SoftAP 网页配网 | **设置页 Wi-Fi 配网 UI** + SoftAP 降级 | 见第 5 节 |
| BMI270 Shake Lab | 同款 IMU | 逻辑可移植 |
| SD 电台/照片/播客 | SDMMC | 逻辑可移植 |
| NABO/表情包 | `nabo_assets` | 状态映射见 manifest |

---

## 4. 建议目录结构

```text
xiaozhi-tab5/
  main/
    boards/
      m5stack-tab5/          # 或 nabo-tab5（独立 SKU）
        config.h
        m5stack_tab5.cc      # Board 组装
        tab5_audio_codec.*
        wifi_provision_ui.*  # 设置内配网页
        nabo_face.*          # NABO 表情模块
        desktop_ui.*         # 可先放 board-local，稳定后再抽 common
        services/            # weather / radio / audio_focus / power
      common/                # 官方公共
    application/ …           # 小智内核（尽量少改）
  components/nofrendo/ …     # 可选后置
  nabo_assets/               # 或 sdcard/nabo、partitions 资源
  sdkconfig.defaults.esp32p4
  partitions/…
```

**独立 SKU 建议：** `nabo-tab5`（`CONFIG_BOARD_TYPE_NABO_TAB5`），避免与官方 `m5stack-tab5` OTA 通道冲突。

---

## 5. 设置页独立 Wi-Fi 配网（推荐做，且适合 Tab5）

### 5.1 结论

**可以，而且应该做成设置页内的完整配网界面。**  
QDTech 小屏主要靠手机 SoftAP 网页；Tab5 是 5" 触摸屏 + 本地键盘，**设备端直连 Wi-Fi 6** 体验更好。SoftAP/手机网页保留为「无屏/忘密/批量」降级通道。

### 5.2 流程

```text
Settings → Wi-Fi
  ├─ 当前状态：未连接 / 已连接 SSID / IP / 信号
  ├─ [扫描网络] → 列表（SSID、锁图标、RSSI、频段提示 Wi-Fi 6）
  ├─ 点选 SSID → 虚拟键盘输密码 → [连接]
  ├─ 连接中… 成功写入 NVS；失败显示原因（超时/密码错）
  ├─ 已保存网络：切换 / 设为默认 / 删除
  └─ [手机配网 SoftAP] 兜底（可选）
```

### 5.3 与小智内核关系

- 复用官方 `WifiStation` 扫描/连接/凭据 NVS，**UI 只做展示与输入**。  
- 连接成功后走现有 `Application` 联网、MQTT/WebSocket、激活流程。  
- 配网页打开时可暂停重扫描风暴，避免抢 C6/SDIO 带宽（和语音任务错峰）。  
- Wi-Fi 6：C6 支持 802.11ax 2.4 GHz；列表可显示 `11ax` 标记，**仍兼容 11n 路由**（家用常见）。

### 5.4 配网 UI 状态机

`Idle → Scanning → List → InputPsk → Connecting → Success | Error → List`

失败可重试；成功自动返回设置页并刷新状态栏。

---

## 6. 桌面信息架构（围绕小智）

| 页面 | 作用 | 第一期 |
|---|---|---|
| **Xiaozhi** | NABO 表情 + 字幕 + 配网引导 | ✅ 核心 |
| **Main** | 时钟/天气/状态栏 | ✅ |
| **Apps** | 磁贴入口 | ✅ |
| **Settings → Wi-Fi** | **独立配网** | ✅ |
| Settings 其它 | 音量/亮度/主题/关于/OTA | ✅ 简版 |
| Radio / Music | 网络电台 | 二期 |
| Photos / Calendar / Focus | SD 应用 | 二期 |
| Shake Lab / Camera | BMI270 / SC2356 | 三期 |

手势：左右滑切换主区页；设置内用列表+返回键（大屏目标更大，优先 `lv_indev`）。

NABO 状态映射：`idle/listen/think/speak/happy/wave/wake/tired/sleep/sad/angry` ← `manifest.json`。

---

## 7. 驱动适配检查表

| 模块 | 动作 |
|---|---|
| 显示 | MIPI-DSI ILI9881C 路径 + ST7123/7121 批次；720p 双缓冲/部分刷新 |
| 触摸 | GT911 或 ST712x TDDI；统一 `lv_indev` |
| 音频 | ES8388 播放 + ES7210 采集；AEC；音量曲线重做（Tab5 旧 log 曾报音量不对） |
| 亮度 | PI4IOE / GPIO22 LEDA；PWM 曲线校准 |
| Wi-Fi | C6 SDIO；扫描/连接 API 与 NVS 对齐 WifiBoard |
| 电源 | INA226 电量、IP2326 充电；关机双击电源键等电源键语义 |
| RTC | RX8130CE 校时、定时唤醒 |
| SD | SDMMC/SPI，长文件名 CP936/UTF-8 |
| 分区 | 16MB flash，双 OTA；资源可放 FATFS 或内置 |

---

## 8. 实施阶段

| 阶段 | 交付 | 验证 |
|---|---|---|
| P0 环境与裸板 | IDF 6.1 + 固定上游提交 + 编译 `nabo-tab5` | build / flash 有日志 |
| P1 人机底座 | 显示+触摸+喇叭+麦+小智 idle/listen/speak + NABO 静态图 | 云端对话可用 |
| P2 **Wi-Fi 设置配网** | 扫描/输密/连接/已存网络 + 状态栏 | 无手机可完成入网 |
| P3 桌面产品 | Main/Apps/Settings 完整 + AudioFocus | 可当桌面终端 |
| P4 媒体扩展 | 电台/照片/播客… | 按需 |

---

## 9. 与「已有资料」的关系

| 已有 | 进入架构的位置 |
|---|---|
| `nabo_assets/` | `NaboFace` 资源，挂 Xiaozhi 页 |
| `manifest.json` | 状态→帧表，生成 `nabo_assets_gen.h` |
| `INTEGRATION.md` | `SetXiaozhiEmotion` 接口草图 |
| QDTech 仓库逻辑 | Services + Desktop 页面模型 + 配网产品化思路 |
| Tab5 BSP / UserDemo | 显示触摸音频参考实现 |

---

## 10. 风险

1. **屏幕批次** ST7123/7121 与旧固件不兼容 → 板级必须探测或配置两套 init。  
2. **P4 无原生 Wi-Fi**，C6 固件/SDIO 异常会导致「配网失败」→ 设置页要暴露 C6/驱动错误。  
3. 720p + 全套 GIF/PSRAM 帧 → 注意 LVGL 缓冲与语音线程栈（PSRAM 栈策略沿用 QDTech）。  
4. 旧 `m5stack-tab5` README 曾记 listening 延迟、亮度/音量不对 → P1 必须先打平再做 UI 炫技。
