# 编辑器界面 (Editor Interface)

HimiiEditor 采用 DockSpace 停靠布局，提供场景编辑、资源管理与脚本开发能力。

## 主要面板

### 1. 视口 (Viewport)

查看与编辑 2D/3D 场景的主区域。

- **右键 + WASD**：漫游（编辑模式下的场景相机）。
- **滚轮**：缩放。
- **Play** 模式下显示运行时画面；**Edit** 模式下可用 **ImGuizmo** 变换选中物体（移动 / 旋转 / 缩放）。

### 2. 场景层级 (Scene Hierarchy)

当前场景中的实体 (Entity) 树。

- **右键**：创建实体（Empty、Camera、Sprite 等）。
- **拖拽**：调整父子层级（若支持）。
- 选中实体后，右侧属性面板显示其组件。

### 3. 属性面板 (Properties)

显示选中实体的组件与参数。

- 编辑 **Transform**、**SpriteRenderer**、**Script Component** 等。
- **Add Component** 添加新组件。
- **Script Component**：填写类名、编辑 public 脚本字段（含 **Entity** 引用下拉）、**Edit** 打开脚本文件。

### 4. 内容浏览器 (Content Browser)

管理项目资源（纹理、脚本、场景等），对应项目目录下的 `assets` 等文件夹。

- 拖拽资源到视口或属性面板使用。
- 可创建 C# 脚本、打开场景文件（`.himii`）等。

## 日志与调试面板

### Console（Window → Console）

**运行时日志**面板，显示游戏脚本通过 `Log.Info` / `Warning` / `Error` 输出的内容。

- **Clear**：清空缓冲。
- **Show Engine Logs**：勾选后同时显示引擎内部日志（默认仅显示脚本 `Log`）。
- **Auto Scroll**：新日志时自动滚到底部。
- 进入 **Play** 时自动清空，避免与上次运行混淆。

### Script Console（Window → Script Console）

**脚本编译**输出，显示 `dotnet build` 的结果。

- 编译中 / 成功 / 失败状态提示。
- 点击带 `文件.cs(行,列): error` 格式的行，可在配置的 IDE 中打开对应脚本（见 [脚本工作流](ScriptWorkflow.md)）。

## 工具栏与运行模式

视口上方工具栏：

- **Play**：运行游戏（复制场景、启动脚本与物理）；再次点击或 **Stop** 返回编辑。
- **Simulate**：仅物理模拟（不运行脚本生命周期）。
- **Stop**：结束 Play / Simulate，恢复编辑场景。

## 菜单要点

| 菜单 | 项 | 说明 |
|------|-----|------|
| **File** | New / Open / Save Project | 项目管理 |
| **File** | Open C# Project | 用配置的 IDE 打开解决方案 |
| **File** | Project Settings | 项目级脚本 IDE 覆盖（`.hproj`） |
| **Edit** | Preferences | 全局默认脚本 IDE（`editor_settings.yaml`） |
| **Window** | Console / Script Console | 见上文 |
| **Window** | Animation / TileMap / Particle 等 | 专项资源编辑器 |

## 脚本 IDE 配置

- **Edit → Preferences**：设置默认 IDE（Visual Studio、VS Code、Rider 或 Custom 可执行文件）。
- **File → Project Settings**：项目可覆盖为「使用编辑器默认」或指定 IDE。

详见 [脚本工作流](ScriptWorkflow.md)。
