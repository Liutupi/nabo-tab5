# NABO-Tab5

**M5Stack Tab5（ST7121）上的小智 AI / NABO 桌面固件工程**

以开源小智（xiaozhi-esp32）语音协议为内核，在 Tab5 大屏上做 NABO 角色桌面、设置页 Wi-Fi 配网与本地应用。架构借鉴 [qdtech-s3-touch-lcd-3.5-xiaozhi-firmware](https://github.com/Liutupi/qdtech-s3-touch-lcd-3.5-xiaozhi-firmware) 的「语音内核 + 板级产品层」分层。

## 硬件

| 项 | 规格 |
|---|---|
| MCU | ESP32-P4 + **ESP32-C6 Wi-Fi 6** |
| 屏幕 | 5" 1280×720 MIPI-DSI **ST7121**（TDDI 触控一体） |
| 音频 | ES8388 + ES7210 双麦 AEC |
| 存储 | 16MB Flash / 32MB PSRAM / microSD |
| 传感器 | BMI270、RX8130CE RTC、INA226 电源监测 |

官方资料：[docs.m5stack.com/en/core/Tab5](https://docs.m5stack.com/en/core/Tab5)

## 功能规划

- [x] 分层架构与硬件适配说明
- [x] NABO 表情状态机（idle/listen/speak/happy/sad/angry…）
- [x] 设置页 Wi-Fi 配网 UI（扫描 / 输密 / 连接）
- [x] SD 资源包约定（见 `docs/SD_AUDIT.md`）
- [ ] 并入 xiaozhi-esp32 后 `idf.py build`
- [ ] ST7121 显示 / 触摸驱动
- [ ] 桌面 Main / Apps / Settings 完整页

## 仓库结构

```text
main/boards/nabo-tab5/   板级：Board / NaboFace / WifiProvisionUi / DesktopUi
partitions/              16MB 分区草图
scripts/                 NABO 资源 → C 头文件生成
docs/                    架构、SD 审计、接入说明
```

## 交接 / Handoff

完整工作状态、构建命令、路径与下一步见 **[HANDOFF.md](HANDOFF.md)**。

**编译里程碑（2026-09-22）：** 官方 `m5stack-tab5` 已在 ESP-IDF 6.0.2 编译成功（`xiaozhi.bin` 3.49MB，app 余 11%），含 ST7121 驱动。

## 快速开始（接入 xiaozhi 树后）

```bash
# ESP-IDF 5.5+
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp32p4;sdkconfig.nabo-tab5.defaults" set-target esp32p4
idf.py build
```

SD 卡：将 `docs/sd-layout` 约定资源（含 `/nabo/`）拷贝到 FAT32 卡根目录。

## 硬件批次

请查看 Tab5 **背部贴纸**：

- **ST7121**（本工程目标）/ ST7123  
- 早期批次为 ILI9881C + GT911（需另路径）

## 许可

MIT（与小智开源项目一致）。角色形象 NABO 版权归作者所有。
