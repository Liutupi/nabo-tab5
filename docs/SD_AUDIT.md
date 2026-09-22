# 开发板 SD 卡资源审计（Tab5 / ST7121）

审计对象：`D:\开发板SD卡`（QDTech v1.8.19 资源包）  
目标板：M5Stack Tab5（ST7121 触控一体屏）+ NABO 小智桌面

---

## 1. 现状总览

| 目录 | 规模 | 状态 | 用途（沿用 QDTech） |
|---|---|---|---|
| `radio.json` | 电台列表 | ✅ 可用 | 网络电台 |
| `books/` | 250 封面 + books.json | ✅ 完整 | 摇摇荐书 |
| `movies/` | 250 海报 + movies.json | ✅ 完整 | 摇摇荐影 |
| `calendar/bone_weight/` | 农历+称骨数据 | ✅ 完整 | 日历称骨 |
| `calendar/zodiac/` | 星座详情 + 12 图 | ✅ 完整 | 星座 |
| `games/puzzle_arcade/` | 8 类关卡 + 封面 | ✅ 完整 | 益智馆 |
| `FC/` | 45 个 .nes | ✅ | FC/NES |
| `roms/md/` | 55+ MD 镜像 | ✅ | MD/retro-go |
| `roms/md-incompatible/` | 问题 ROM | ✅ 隔离正确 | 不加载 |
| `podcast/` | 磁盘 **160** 个 mp3，但 `index.json` 只登记 **80** 集 | ⚠️ 索引不全（约 3.7GB） | 播客 |
| `shake_lab/` | 骰子精灵 + 占卜图 | ✅ | 摇摇实验室 |
| `wooden_fish/` | background.rgb565 | ✅（240×160） | 木鱼 |
| `PHOTOS/` | **仅 1 张模板图** | ❌ 缺内容 | 照片轮播 |
| `retro-go/config/` | **空** | ⚠️ 缺 | MD 交接配置 |
| `nabo/` | **不存在** | ❌ 缺 | NABO 表情/动画 |

根目录说明与校验：`SD卡使用说明-v1.8.19.txt`、`SD卡文件校验-v1.8.19.csv`（仍标注 QDTech，需出 Tab5 版）。

---

## 2. 必须补充（Tab5 第一期）

### 2.1 `nabo/` 表情资源（核心）

从 `D:\tab5\nabo_assets\` 同步到 SD：

```text
/nabo/
  manifest.json
  full/   idle_*.png speak_*.png …   # 512×512
  bust/   blink_*.png                # 384×384
```

建议：主状态帧放 SD（省 Flash）；若要离线秒开，可将 `idle_01` + `speak_*` 编入固件。

### 2.2 `PHOTOS/` 示例照片

- 至少 3–5 张 JPG，建议 **1280×720 或 960×640**，文件名 `001.jpg`…  
- 删除或移走「请按照这张图的分辨率…」说明图，避免轮播进 UI  
- Tab5 720p 比 480×320 更适合直接用 16:9 图

### 2.3 清理垃圾文件

- `calendar/` 下 **14 个** `._*` / `.DS_Store`（macOS 垃圾）  
- 根目录 `radio-v1.8.6.json` 与 `radio.json` 重复 → 保留 `radio.json` 即可

### 2.4 `retro-go/config/`

按 v1.8.19 说明用于 MD 主系统交接。若 Tab5 第一期不做 MD，可暂空；做 MD 时补 `md_handoff` 相关约定（**不要**手动放固件 bin）。

---

## 3. 优化建议（可第二期）

| 项 | 现状 | 建议 |
|---|---|---|
| 封面/海报分辨率 | 220×300 | 列表够用；详情页可预留给 720p 生成 440×600 可选图（非必须） |
| 播客封面 | jpg/png/jpeg 混用 | 统一 jpg 可减句柄/解码差异；已有 index.json 可继续用 |
| `wooden_fish/background.rgb565` | 240×160 | Tab5 可升 480×320 或 720×360（需固件同步改读取） |
| `shake_lab` 骰子 | `rgb565/argb8888` | 分辨率按 Tab5 UI 重导出一版（防糊） |
| 说明文档 | QDTech v1.8.19 | 改写为 `SD卡使用说明-Tab5-ST7121.txt` |
| 校验 CSV | 旧包 | 资源变更后重生成 SHA256 清单 |
| 中文文件名 | FC/MD 有中文名 | 保持 FAT 长文件名；固件继续 UTF-8/CP936 兼容（QDTech 已有经验） |

---

## 4. 与固件路径约定（Tab5）

| 功能 | SD 路径 | 备注 |
|---|---|---|
| NABO 表情 | `/nabo/` | **新增** |
| 电台 | `/radio.json` | 已有 |
| 照片 | `/PHOTOS/` | 补图 |
| 播客 | `/podcast/` + `index.json` | 已有 |
| 书籍 | `/books/` | 已有 |
| 电影 | `/movies/` | 已有 |
| 日历 | `/calendar/` | 已有 |
| 益智 | `/games/puzzle_arcade/` | 已有 |
| FC | `/FC/` | 已有 |
| MD ROM | `/roms/md/` | 已有 |
| 摇摇 | `/shake_lab/` | 已有 |
| 木鱼 | `/wooden_fish/` | 已有 |

挂载：SDMMC 或 SPI 均可；中文名需长文件名。

---

## 5. 第一期建议只启用（避免一次全上）

**P1 就绪：** `nabo/` + `PHOTOS/` + 清垃圾 + 语音小智  
**P2：** `radio.json` 电台、Settings 配网  
**P3：** books/movies/calendar/shake_lab/wooden_fish  
**P4：** FC / MD（retro-go，需双模式刷机意识）

---

## 6. 待办勾选

- [x] 同步 `nabo_assets` → SD `/nabo/`（已生成 `D:\开发板SD卡\nabo\`）
- [ ] 核对 podcast：磁盘 160 mp3 vs index 80 条，补全或清理多余文件
- [ ] 补 PHOTOS 示例 1280×720 JPG
- [ ] 删除 `._*` / `.DS_Store`
- [ ] 精简 `radio-v1.8.6.json`（可选）
- [ ] 更新使用说明为 Tab5/ST7121
- [ ] 重生成校验清单（可选）
