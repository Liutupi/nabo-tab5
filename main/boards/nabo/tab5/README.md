# NABO Tab5 板级 overlay

本目录在 XiaoZhi 官方 Tab5 板级实现上增加 NABO 桌面组件，目标硬件为 ESP32-P4 + ESP32-C6 的 M5Stack Tab5。

## 内容

```text
nabo_tab5.cc             板级组装、面板探测、触摸、电源与摄像头
tab5_audio_codec.*       ES8388 + ES7210 音频
esp_lcd_st7121.*         ST7121 面板驱动
esp_lcd_st7123.*         ST7123 面板驱动
config.h                 引脚与面板初始化表
config.json              P4 / P4X 双构建变体
desktop_ui.*             NABO 页面容器（待挂入显示生命周期）
wifi_provision_ui.*      屏内 Wi-Fi 配网组件（待接网络服务）
nabo_face.*              NABO 表情状态机（待接 SD/LVGL 文件系统）
nabo_assets_gen.h        由 assets/nabo/manifest.json 生成
```

不要手工复制并修改 XiaoZhi 的 Kconfig/CMake。请从本仓库根目录运行：

```bash
python scripts/apply_to_xiaozhi.py work/xiaozhi-esp32
```

随后在 XiaoZhi 树内构建：

```bash
python scripts/build.py nabo/tab5 --name nabo-tab5
# P4 Rev >= 3：
python scripts/build.py nabo/tab5 --name nabo-tab5-p4x
```

当前产品层组件会参与编译，但尚未在 `NaboTab5Board` 中创建；这样可以先保持官方硬件启动路径稳定，再通过真机验证逐层接入。
