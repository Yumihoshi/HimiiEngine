# Sprite Animation（SpriteFrames）

## 概述

2D 逐帧动画采用 **Godot SpriteFrames** 风格：一个 **`.anim`** 资产共享图集，内含多条 **命名动画**（如 `walk`、`idle`）。运行时由 **Sprite Animation** 组件的 **Current Animation** 选择播放哪一条，配合 **Sprite Renderer** 绘制。

## 工作流

1. **Window → Animation Editor** 或双击内容浏览器中的 `.anim`
2. **已切好的图集**：拖入 PNG → 设置 **Cell Size** → 在 **Animations** 列表选中一条动画 → 在 **Atlas Picker** 上按播放顺序逐格点击（无需 Slice）
3. 用 **Add / Rename / Delete** 管理多条命名动画；每条动画有独立的 **FPS**、**Loop Mode** 与时间轴
4. 可选 **Fill All Cells** 一次性按网格填满当前动画；或用 **Frame List** 拖入 Sprite
5. 保存（Ctrl+S 会 Import 到 AssetManager）
6. 实体添加 **Sprite Animation** + **Sprite Renderer**，指定 Animation 引用，在 Inspector 选择 **Current Animation**
7. 脚本：`Play("walk")`、`CurrentAnimation = "idle"`；编辑时可勾选 **Preview In Scene**

旧版单条 `.anim`（`AssetType: SpriteAnimation`）会自动迁移为名为 `default` 的一条动画。

## Loop Mode

| 模式 | 行为 |
|------|------|
| Loop | 循环播放 |
| Once | 播放到最后一帧后停止 |
| PingPong | 往返播放 |

## 注意

- 必须同时存在 **Sprite Renderer**，否则无法绘制
- 图集模式与帧列表模式互斥；Slice Grid 会切换到图集模式
