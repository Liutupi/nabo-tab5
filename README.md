# NABO Tab5

[![Build NABO Tab5](https://github.com/Liutupi/nabo-tab5/actions/workflows/build.yml/badge.svg)](https://github.com/Liutupi/nabo-tab5/actions/workflows/build.yml)

面向 M5Stack Tab5 的 XiaoZhi 板级 overlay：在官方硬件支持之上扩展 NABO 角色桌面、屏内 Wi-Fi 配网和大屏本地应用。

## 当前能力

- 官方 Tab5 硬件基座：ILI9881C、ST7121、ST7123 自动探测，触控、ES8388 + ES7210 音频、摄像头和电源管理
- 两个构建变体：`nabo-tab5`（P4 Rev < 3）与 `nabo-tab5-p4x`（P4 Rev >= 3）
- NABO 表情状态机与可重复生成的资源索引
- 带 Wi-Fi 列表、密码软键盘和异步连接结果的设置页组件
- 固定 XiaoZhi 上游提交，GitHub Actions 使用 ESP-IDF 6.1 编译两个固件变体

双变体 CI 已通过，[2026-09-23 构建](https://github.com/Liutupi/nabo-tab5/actions/runs/35837740853)还验证了 Rev 1.x 专用 ISP 库的实际链接。`nabo-tab5` 已在 P4 Rev 1.3 真机启动至配网模式，摄像头自检成功抓帧且色彩矩阵越界报错消失；C6 固件版本仍有警告，显示/触摸交互和实际拍照画质尚待人工验收，详见 [HANDOFF.md](HANDOFF.md)。

产品层尚在接线阶段：桌面、表情和屏内配网页仍需挂到 XiaoZhi 的实际页面、SD/LVGL 文件系统和网络服务。详细状态见 [HANDOFF.md](HANDOFF.md)。

## 硬件

| 项 | 规格 |
|---|---|
| MCU | ESP32-P4 + ESP32-C6 Wi-Fi 6 |
| 屏幕 | 5 英寸 720×1280 MIPI-DSI；ILI9881C / ST7121 / ST7123 |
| 音频 | ES8388 + ES7210 双麦 AEC |
| 存储 | 16MB Flash / 32MB PSRAM / microSD |
| 传感器 | BMI270、RX8130CE RTC、INA226 电源监测 |

官方资料：[M5Stack Tab5](https://docs.m5stack.com/en/core/Tab5)

## 仓库结构

```text
main/boards/nabo/tab5/   板级实现与产品层组件
assets/nabo/             NABO 资源清单
partitions/              16MB A/B OTA 分区
scripts/                 overlay 安装与资源生成工具
docs/                    架构、SD 审计、接入说明
.github/workflows/       双变体固件构建
```

## 快速构建

安装并激活 ESP-IDF 6.1 后，从本仓库根目录执行：

```bash
git clone https://github.com/78/xiaozhi-esp32.git work/xiaozhi-esp32
git -C work/xiaozhi-esp32 checkout 4632dc51f0a5ad26e08542e131e6e48da41e4ff3
python scripts/apply_to_xiaozhi.py work/xiaozhi-esp32

cd work/xiaozhi-esp32
python scripts/build.py nabo/tab5 --name nabo-tab5
# P4 Rev >= 3 使用：
# python scripts/build.py nabo/tab5 --name nabo-tab5-p4x
```

上游地址、提交和 IDF 基线记录在 `upstream.json`。overlay 安装脚本默认拒绝其他提交，升级前请先评估上游板级变化。

## 生成 NABO 资源索引

```bash
python scripts/gen_nabo_lvgl.py assets/nabo/manifest.json
```

SD 卡上的资源放在 `/nabo/full/`、`/nabo/bust/` 与 `/nabo/manifest.json`。大体积图片和媒体文件不进入 Git。

## 许可

代码采用 MIT 许可证。NABO 角色形象版权归作者所有。
