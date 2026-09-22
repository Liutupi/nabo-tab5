# NABO Tab5 推进状态

> 2026-09-22 更新。更完整的恢复步骤与风险见 [HANDOFF.md](HANDOFF.md)。

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

## 自动验证

- [x] Python 脚本语法检查
- [x] 资源 manifest 可重复生成 `nabo_assets_gen.h`
- [x] overlay 可安装到固定 XiaoZhi 源码树
- [x] XiaoZhi `build.py` 可识别 `nabo-tab5` / `nabo-tab5-p4x`
- [x] GitHub Actions 固件全量编译绿灯（`nabo-tab5` / `nabo-tab5-p4x`）
- [ ] Tab5 真机烧录与硬件验收

## 下一阶段

1. 将 `DesktopUi` 与 `NaboFace` 接入 `Application` / `LcdDisplay` 生命周期。
2. 挂载 SD 并注册 LVGL 文件系统，使 `/sdcard/nabo/` PNG 可显示。
3. 将 `WifiProvisionUi` 对接实际网络服务，并把扫描移出 LVGL 线程。
4. 统一横竖屏、触摸坐标和摄像头方向。
5. 真机验证显示、触摸、音频、C6、SD、摄像头和电源路径。
