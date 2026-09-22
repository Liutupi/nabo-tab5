# SD 卡目录约定（Tab5 / NABO）

将下列内容拷贝到 **FAT32** microSD 根目录（合并复制，勿删个人照片/存档）。

```text
/
├── nabo/                      # NABO 表情（本仓库 scripts 可生成）
│   ├── manifest.json
│   ├── full/                  # 512×512 全身帧
│   └── bust/                  # 384×384 眨眼胸像
├── radio.json                 # 网络电台
├── PHOTOS/                    # 照片轮播 JPG（建议 1280×720）
├── podcast/
│   ├── index.json
│   ├── epXXX.mp3 / .jpg / .txt
├── books/                     # 荐书
├── movies/                    # 荐影
├── calendar/                  # 称骨 / 星座
├── games/puzzle_arcade/       # 益智馆
├── shake_lab/                 # 骰子 / 占卜
├── wooden_fish/               # 木鱼背景
├── FC/                        # FC ROM
└── roms/md/                   # MD ROM（retro-go）
```

注意：

- 不要把固件 `*.bin` 放进 SD。
- 中文文件名需要长文件名（FAT LFN）。
- 详细缺口与优化见 [`../SD_AUDIT.md`](../SD_AUDIT.md)。
