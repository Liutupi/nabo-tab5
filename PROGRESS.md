# NABO Tab5 推进状态

> 2026-09-23 更新。更完整的恢复步骤与风险见 [HANDOFF.md](HANDOFF.md)。

## 真机触控 / 滑动联调（2026-09-23 晚）

**现状**：本地 `xiaozhi-esp32` 已出 `nabo-tab5` 并烧录 COM5；**横滑切页已通**；点触曾错位，已修「双重旋转」并再烧录，待人工确认点得准不准。

### 滑动无反应 → 已修
根因（`main/boards/nabo/tab5/desktop_ui.*` 等）：
1. 页面挂了 `LV_OBJ_FLAG_GESTURE_BUBBLE`，手势被弹到 screen，页面收不到 `LV_EVENT_GESTURE`。
2. `face_host` / `icon_box` 等可点击层未 `EVENT_BUBBLE`，吞掉按压事件。
3. 装饰控件仍可滚动，可能变成 `scroll_obj` 抑制 gesture。
4. 竖向位移阈值过严；GESTURE + RELEASED 双路径会连切两次页。

修复：页面自身处理手势；装饰层 `pass_through`、按钮 `bubble_to_parent`；触摸 read_cb **直接识别横滑**并切页；主页左右横滑进应用、子页横滑返回；400ms 冷却；TP_INT 改 TIMER 轮询。

### 点触错位 → 已修（待确认）
根因：LVGL 9 `indev_pointer_proc()` 会调用 `lv_display_rotate_point()`；read_cb 又手动转了一次 → **双重旋转**。

修复：read_cb 只交 **原始面板坐标**，由 LVGL 做 UI 坐标变换；滑动位移与 `lv_display_rotate_point` 对齐。

### 本地下次验收
- [ ] 点「菜单」/ 应用磁贴 / 返回是否点得准（若仍偏：记录「点哪里→响应哪里」）
- [ ] 横滑切换回归
- [ ] 音量/亮度滑条拖动

## 已完成

- [x] 以固定 XiaoZhi 上游提交的官方 Tab5 实现替换早期板级骨架
- [x] ILI9881C / ST7121 / ST7123 显示触摸自动探测
- [x] ES8388 + ES7210 音频、摄像头、电源与背光板级实现
- [x] P4 Rev < 3 与 P4X 双构建变体
- [x] 16MB Flash A/B OTA 分区修正
- [x] NABO 表情映射、首帧、one-shot 与播放节拍修复
- [x] Wi-Fi 密码软键盘与异步连接结果接口
- [x] 跨平台资源生成器、上游锁定文件和 overlay 安装脚本
- [x] GitHub Actions 双变体构建与固件上传
- [x] Rev 1.3 真机启动冒烟（见 HANDOFF 第 8 节）
- [x] 本地真机：横滑切页；点触双重旋转修复

## 自动验证

- [x] Python 脚本语法检查
- [x] 资源 manifest 可重复生成 `nabo_assets_gen.h`
- [x] overlay 可安装到固定 XiaoZhi 源码树
- [x] XiaoZhi `build.py` 可识别 `nabo-tab5` / `nabo-tab5-p4x`
- [x] GitHub Actions 固件全量编译绿灯（`nabo-tab5` / `nabo-tab5-p4x`）
- [x] 本地 `build.py nabo/tab5 --name nabo-tab5` + COM5 烧录
- [ ] Tab5 触摸点控精度 / 滑动手势完整验收

## 下一阶段

1. 确认点触坐标（见上）；必要时按「点哪里→响应哪里」再校正旋转/镜像。
2. 将 `DesktopUi` 与 `NaboFace` 接入 `Application` / `LcdDisplay` 生命周期。
3. 挂载 SD 并注册 LVGL 文件系统，使 `/sdcard/nabo/` PNG 可显示。
4. 将 `WifiProvisionUi` 对接实际网络服务，并把扫描移出 LVGL 线程。
5. 统一横竖屏、触摸坐标和摄像头方向。
6. 真机验证显示、音频、C6、SD、摄像头和电源路径。
