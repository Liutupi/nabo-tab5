# HANDOFF — NABO Tab5 / 小智交接本

> 更新：2026-09-23
>
> 仓库：https://github.com/Liutupi/nabo-tab5
>
> 目标硬件：M5Stack Tab5（ESP32-P4 + ESP32-C6）

## 0. 冻结快照

- 标签：[tab5-rev1-smoke-2026-09-23](https://github.com/Liutupi/nabo-tab5/releases/tag/tab5-rev1-smoke-2026-09-23)，固定本次代码、构建脚本和交接记录。
- 预发布包附带 `nabo-tab5-rev1-merged.bin`，来自已在 ESP32-P4 Rev 1.3 真机烧录验证的提交 `fcd3154`；SHA-256 见第 8 节。该固件**只适用于 Rev < 3**，不是功能全部验收的正式版。
- 烧录合并固件会覆盖设备配置分区，原有 Wi-Fi 等设置需要重新配置。Rev >= 3 设备请使用 CI 的 `nabo-tab5-p4x` 变体，不要烧录上述 Rev 1 固件。

## 1. 当前结论

仓库已经从“板级草图”升级为**可安装到 XiaoZhi 源码树的板级 overlay**：

- 硬件实现基于 XiaoZhi 当前官方 Tab5 板级代码，覆盖 ILI9881C、ST7121、ST7123 自动探测、触控、ES8388 + ES7210 音频、摄像头、充电与背光。
- 支持 `nabo-tab5`（P4 Rev < 3）和 `nabo-tab5-p4x`（P4 Rev >= 3）两个构建变体。
- 上游源码固定到 `upstream.json` 中的提交，CI 使用 ESP-IDF 6.1 构建两个变体，避免“今天能编、明天上游变了不能编”。
- 修正了 16MB Flash 分区、LCD_RST 释放方式、NABO 表情错位/漏帧、首帧跳过、单帧一次性动画无法结束等问题。
- 配网页增加软键盘、连接中状态和异步连接结果回调；资源生成器改为跨平台、可校验的命令行工具。
- GitHub Actions [运行 #35837740853](https://github.com/Liutupi/nabo-tab5/actions/runs/35837740853) 已验证两个变体均可在 ESP-IDF 6.1 下完整编译，并检查 Rev 1.x 固件确实链接专用 ISP 库。
- `nabo-tab5` 固件已烧录到 ESP32-P4 Rev 1.3 真机并完成启动冒烟测试，见第 8 节。
- SC202CS 色彩矩阵越界已通过低色温配置修正和 ISP 最终写入限幅解决；真机复测抓帧成功且不再报错。

边界也要说清楚：`NaboFace`、`DesktopUi` 和屏内 Wi-Fi 配网页目前已经能参与编译，但**还没有挂入 XiaoZhi 的实际页面生命周期与网络服务**；SD 卡挂载和 LVGL 文件系统桥也仍需完成。真机已确认各外设初始化，但屏幕实际画面、触摸操作、扬声器/麦克风音质、拍照画质和联网到路由器尚未验收。

## 2. 固定基线

| 项目 | 基线 |
|---|---|
| XiaoZhi | `78/xiaozhi-esp32`，提交见 `upstream.json` |
| ESP-IDF | 6.1（上游最低要求 6.0.1） |
| LVGL | 由固定的 XiaoZhi 提交锁定 |
| Flash | 16MB |
| PSRAM | 32MB |
| 屏幕 | 5 英寸 720×1280 MIPI-DSI；应用可另做横屏布局 |
| 网络 | ESP32-C6 + esp-hosted SDIO |

`apply_to_xiaozhi.py` 还会把 Espressif 的 `esp_ipa` 修复提交复制到 XiaoZhi 的
`components/espressif__esp_ipa/`，由 ESP-IDF 作为本地组件优先使用。修复提交为
`5ec4d12101d7308dd9f826ff211312387f63e22d`。原因是组件仓库中的 2.4.0
发布包尚未包含 ESP-IDF 6.x + P4 Rev 1.x 专用 ISP 库，启用 SC202CS 时会执行
Rev 3+ 指令并触发 `Illegal instruction`。

安装脚本也会把同一固定提交的 `esp_video` 复制为本地组件，在 CCM 写入硬件前对自动白平衡叠加后的系数限幅至 `[-3.99, 3.99]`；同时从该提交生成 SC202CS 专用配置，修正默认低色温矩阵中超限的 `4.5445`。这两层修正分别处理配置原值和运行时增益，不能只保留前者。

早期在 ESP-IDF 6.0.2 下成功编译官方 `m5stack-tab5` 的记录仅作为历史参考；当前应以 `upstream.json` 和 GitHub Actions 为准。

## 3. 从零构建

需要 Git、Python 3、ESP-IDF 6.1。以下命令从 `nabo-tab5` 仓库根目录执行：

```bash
git clone https://github.com/78/xiaozhi-esp32.git work/xiaozhi-esp32
git -C work/xiaozhi-esp32 checkout 4632dc51f0a5ad26e08542e131e6e48da41e4ff3

python scripts/apply_to_xiaozhi.py work/xiaozhi-esp32

cd work/xiaozhi-esp32
python scripts/build.py nabo/tab5 --name nabo-tab5
# P4 Rev >= 3 / P4X 设备改用：
# python scripts/build.py nabo/tab5 --name nabo-tab5-p4x
```

脚本会把 `main/boards/nabo/tab5/` 安装到 XiaoZhi，并自动注册 Kconfig 与 CMake。默认拒绝未锁定的上游提交；升级上游时应先评估差异，再更新 `upstream.json`。重复覆盖测试树可加 `--force`。

烧录：

```bash
cd work/xiaozhi-esp32
idf.py -p <串口> flash monitor
```

设备进入下载模式：长按 Reset 约 2 秒，绿灯快闪后松开。

## 4. 本次完成内容

### 板级与构建

- [x] 板目录规范化为 `main/boards/nabo/tab5/`
- [x] 接入官方 Tab5 显示、触摸、音频、摄像头和电源管理实现
- [x] ST7121 / ST7123 / ILI9881C 自动探测
- [x] 按 M5Stack 要求，LCD_RST 拉低复位后释放为输入上拉，降低共享 I2C 不稳定风险
- [x] 双 P4 版本构建配置
- [x] 固定上游提交与 ESP-IDF 版本
- [x] GitHub Actions 双变体编译与固件产物上传

首次绿色构建记录：

| 变体 | `xiaozhi.bin` | 4MB app 分区余量 | `merged-binary.bin` |
|---|---:|---:|---:|
| `nabo-tab5` | `0x387410` | `0x68bf0`（10%） | `0xad8f49` |
| `nabo-tab5-p4x` | `0x38d030` | `0x62fd0`（10%） | `0xad8f49` |

### 存储与资源

- [x] 分区修正为 16MB：NVS + OTA data + 双 4MB OTA 槽 + 8MB assets
- [x] `assets/nabo/manifest.json` 成为表情资源表的唯一来源
- [x] 资源生成脚本支持任意平台、输出路径和路径前缀，并校验非法文件名、FPS 与状态冲突
- [x] 表情按名字映射，不再因 `blink` 插入顺序导致 tired/sleep/sad/angry 错位
- [x] 修复首次 idle 不显示、非循环动画末帧反复刷新和单帧 one-shot 不结束

### 配网界面

- [x] Wi-Fi 列表、密码框、LVGL 软键盘和连接 spinner
- [x] 连接操作改为异步发起，由网络任务调用 `SetConnectionResult()` 回写结果
- [x] 隐藏页面或连接结束时清空密码缓冲

## 5. 下一步（按优先级）

### P0：真机验收

1. 已识别本机 ESP32-P4 Rev 1.3，应使用 `nabo-tab5`；Rev >= 3 设备才用 `nabo-tab5-p4x`。
2. 已烧录提交 `fcd3154` 的 CI 产物并确认启动至配网模式，详见第 8 节。
3. 请人工验证实际显示、触摸坐标、背光、扬声器/双麦、拍照画质、连接路由器与休眠唤醒。
4. SC202CS 默认 IPA 配置中的越界系数 `4.5445` 和自动白平衡增益叠加后的越界已修正；复测 22 秒内自检抓帧 150 帧、CCM 错误 0。仍需用实际样张验证颜色与暗光表现。
5. 核查 esp-hosted 提示的 Host `2.12.0` / C6 co-processor `0.0.0` 版本不一致；当前 SDIO 初始化及热点启动成功，升级 C6 前先确认实际固件版本和官方升级流程。
6. 特别观察 ST7121/ST7123 设备上 I2C 是否仍出现复位或超时。

### P1：接通产品层

1. 把 `DesktopUi` 挂到官方 `LcdDisplay` 的创建与销毁生命周期。
2. 把 `Application` 的 listening/thinking/speaking/error 状态送给 `NaboFace::OnDeviceState()`。
3. 挂载 microSD，并为 LVGL 注册文件系统驱动；确认 `/sdcard/nabo/*.png` 能被解码显示。
4. 将 `WifiProvisionUi` 的扫描/连接回调接到 XiaoZhi 网络服务，并把 LVGL 更新切回 UI 线程。
5. 将同步扫描改成工作线程任务，避免扫描期间触摸和动画短暂停顿。

### P2：大屏体验与可靠性

1. 明确竖屏 720×1280 或横屏 1280×720 产品方向，统一显示旋转、触摸变换和摄像头方向。
2. 表情资源按实际显示尺寸压缩，优先测试 RGB565/分块解码，避免 PNG 解码造成帧抖动。
3. 加入 SD 缺失、资源损坏、C6 未启动、网络连接超时等可见降级页面。
4. 补硬件在环冒烟测试与版本信息页（固件提交、上游提交、面板类型、P4 revision）。

## 6. 已知限制与坑

- 这是 overlay 仓库，不是可直接运行 `idf.py build` 的完整 ESP-IDF 工程。
- `LoadManifest()` 当前只探测 SD manifest 是否存在，运行时 JSON 覆盖尚未实现；编译期资源表来自生成头文件。
- `lv_image_set_src()` 使用 `/sdcard/...` 前，必须先完成 SD 挂载与 LVGL 文件系统桥；否则表情路径存在但图片不会显示。
- 屏内 Wi-Fi 扫描回调当前仍是同步接口；连接接口已异步化。
- P4 没有原生 Wi-Fi。联网问题优先检查 C6 电源、esp-hosted 固件与 SDIO 引脚，不要只查普通 ESP Wi-Fi 配置。
- 在 Espressif 发布包含 less-v3 ISP 库的新版本前，不要移除本地 `esp_ipa` 组件；升级后需在 Rev 1.3 真机上复测 SC202CS 初始化。CI 会检查 Map 文件确实链接到 less-v3 库。
- SC202CS 色彩矩阵报错已在 Rev 1.3 上消失，但只验证了初始化抓帧，尚未完成实际样张画质验收。升级 `esp_video` / `esp_ipa` / `esp_cam_sensor` 时需重新检查这个兼容补丁是否仍必要。
- C6 目前能通过 SDIO 启动配网热点，但日志仍提示 co-processor `0.0.0` 版本不匹配，尚未验证连入家庭 Wi-Fi 的稳定性。
- 不要提交 `build/`、`managed_components/`、`sdkconfig` 或大体积 SD 媒体包。

## 7. 资源与状态映射

| XiaoZhi / 产品状态 | NABO clip | 说明 |
|---|---|---|
| idle | idle | 循环 |
| listening | listen | 定格在末帧 |
| thinking / connecting / upgrading | think | 定格在末帧 |
| speaking | speak | 口型循环 |
| error | sad | 定格 |
| 主动动作 | wave / wake | one-shot 后回到目标状态 |

`blink` 是辅助片段，不占用主情绪枚举位置。

## 8. 2026-09-23 真机验收记录

| 项目 | 结果 |
|---|---|
| 设备 | ESP32-P4 Rev 1.3，USB-Serial/JTAG；32MB PSRAM |
| 固件 | `fcd31540d1a876d8cd47a1033cbcfa6979621229`，`nabo-tab5`，合并固件 SHA-256 `4bbcef371a2c2598e21cdabbfc2d71efeabea0f239ff09fb65032b6ede21f6ec`；[CI #35837740853](https://github.com/Liutupi/nabo-tab5/actions/runs/35837740853) 双变体通过 |
| 烧录 | `/dev/cu.usbmodem212401`，写入后哈希校验通过 |
| 屏幕/触摸 | 检出 ST7121 面板、720×1280 显示驱动和 ST7123 触摸控制器（最多 10 点）；实际视觉/触摸操作待人工确认 |
| 摄像头 | 检出 SC202CS `PID=0xeb52`；自检 5 秒抓取 150 帧；22 秒启动观察中 CCM 越界报错 0，原先 `Illegal instruction` 不再发生；实际样张待测 |
| 音频 | 双工通道创建、音频编解码器启动；音质/双麦待人工确认 |
| C6/网络 | SDIO 初始化成功，进入 `wifi_configuring`，热点及配置 Web 服务启动；路由器连接待测，且有 Host/C6 版本不一致警告 |
| 稳定性 | 最终固件复位后观察 22 秒，无 Guru Meditation 或自动重启；前一版固件在配网状态继续运行数分钟无重启；最终固件长时运行待测 |

### 后续验收记录模板

```text
设备背贴/批次：
构建变体：nabo-tab5 / nabo-tab5-p4x
固件提交：
面板探测：ILI9881C / ST7121 / ST7123
显示与触摸：
扬声器/双麦：
摄像头：
C6/联网：
SD/NABO 表情：
异常串口日志：
```
