const fs = require('fs');

const results = JSON.parse(fs.readFileSync('D:/Himii-Engine/.understand-anything/tmp/ua-file-extract-results-3.json', 'utf8'));
const input = JSON.parse(fs.readFileSync('D:/Himii-Engine/.understand-anything/tmp/batch-input-3.json', 'utf8'));
const batchImportData = input.batchImportData;

// Summary & tag definitions for each file
const fileMeta = {
  "HimiiEditor/src/CamerController.h": {
    summary: "定义编辑器摄像机控制器类 CameraController，通过键盘输入（WASD/QE）控制摄像机在场景中的移动和旋转，挂载为脚本组件运行。",
    tags: ["camera", "editor", "input-handling", "component"],
    complexity: "simple",
    languageNotes: "使用 Himii 引擎的 Native Script 组件模式，通过 GetComponent 访问 TransformComponent。"
  },
  "HimiiEditor/src/EditorExternalFileDrop.cpp": {
    summary: "实现编辑器外部文件拖放功能，将拖入窗口的文件路径存储到内部队列中供 EditorLayer 消费处理。",
    tags: ["file-drop", "editor", "glfw-integration", "utility"],
    complexity: "simple"
  },
  "HimiiEditor/src/EditorExternalFileDrop.h": {
    summary: "声明编辑器外部文件拖放接口类，封装 GLFW 的 drop callback，提供 Install 和 ConsumePendingPaths 两个静态方法。",
    tags: ["file-drop", "editor", "interface", "glfw-integration"],
    complexity: "simple"
  },
  "HimiiEditor/src/EditorLayer.cpp": {
    summary: "编辑器核心层的完整实现，管理启动画面、项目中心、主菜单栏、视口渲染、ImGuizmo 变换工具、Tilemap 编辑、场景播放/模拟、脚本编译、撤销/重做、文件拖放和布局持久化等全部编辑器功能。",
    tags: ["editor", "entry-point", "ui-layer", "viewport", "tilemap-editor", "scene-management"],
    complexity: "complex",
    languageNotes: "2511 行超大型文件，包含 49 个成员函数，深度集成 ImGui 停靠布局、ImGuizmo 操纵器和 Renderer2D/Renderer3D 覆盖层绘制。"
  },
  "HimiiEditor/src/EditorLayoutDefaults.cpp": {
    summary: "实现编辑器默认停靠布局的构建逻辑，在首次启动或布局文件缺失时自动生成包含视口、场景层级、属性面板等窗口的默认布局。",
    tags: ["editor", "layout", "docking", "initialization"],
    complexity: "moderate"
  },
  "HimiiEditor/src/EditorLayoutDefaults.h": {
    summary: "声明编辑器默认布局工具函数的头文件，包括 ApplyDefaultDockLayoutIfNeeded 和布局检测辅助函数。",
    tags: ["editor", "layout", "docking", "interface"],
    complexity: "simple"
  },
  "HimiiEditor/src/TilemapEditorUtility.cpp": {
    summary: "实现 Tilemap 编辑器的数学工具函数集，包括视口鼠标坐标到世界坐标的投影变换、Tile 坐标计算和图集 UV 映射等。",
    tags: ["tilemap", "utility", "math", "coordinate-transform", "editor"],
    complexity: "moderate",
    languageNotes: "使用 glm 数学库进行逆矩阵投影变换和射线-平面求交计算。"
  },
  "HimiiEditor/src/TilemapEditorUtility.h": {
    summary: "声明 TilemapEditorUtility 工具类，提供视口坐标转换、Tile 坐标计算和图集 UV 映射等静态方法。",
    tags: ["tilemap", "utility", "math", "interface", "editor"],
    complexity: "simple"
  },
  "HimiiEditor/src/commands/EditorCommandHistory.cpp": {
    summary: "实现编辑器命令历史管理器，维护 Undo/Redo 双栈结构，支持命令执行、撤销、重做和合并操作，带最大深度限制。",
    tags: ["undo-redo", "command-pattern", "editor", "history"],
    complexity: "moderate"
  },
  "HimiiEditor/src/commands/EditorCommandHistory.h": {
    summary: "声明编辑器命令历史类 EditorCommandHistory，提供 Execute/Undo/Redo/Clear 接口和 CanUndo/CanRedo 状态查询。",
    tags: ["undo-redo", "command-pattern", "editor", "history"],
    complexity: "simple"
  },
  "HimiiEditor/src/commands/EditorCommands.cpp": {
    summary: "实现五种具体的编辑器命令：CreateEntityCommand、DeleteEntityCommand、ModifyTransformCommand、ModifyUITransformCommand 和 ModifyEntityTagCommand，均遵循 IEditorCommand 接口。",
    tags: ["undo-redo", "command-pattern", "entity", "transform", "editor"],
    complexity: "moderate",
    languageNotes: "使用实体快照（EntitySnapshot）进行序列化/反序列化以支持实体的创建和删除撤销。"
  },
  "HimiiEditor/src/commands/EditorCommands.h": {
    summary: "声明五种编辑器命令类及其 Execute/Undo/TryMerge 接口，涵盖实体增删、Transform/UITransform 修改和 Tag 修改操作。",
    tags: ["undo-redo", "command-pattern", "entity", "transform", "interface"],
    complexity: "moderate"
  },
  "HimiiEditor/src/commands/EntitySnapshot.cpp": {
    summary: "实现实体的 YAML 序列化快照功能，通过 SceneSerializer 将实体序列化为 YAML 字符串以便在撤销操作中恢复。",
    tags: ["serialization", "entity", "snapshot", "undo-redo", "yaml"],
    complexity: "simple"
  },
  "HimiiEditor/src/commands/EntitySnapshot.h": {
    summary: "声明 EntitySnapshot 工具类，提供 Serialize 和 Restore 两个静态方法用于实体的序列化快照和恢复。",
    tags: ["serialization", "entity", "snapshot", "interface"],
    complexity: "simple"
  },
  "HimiiEditor/src/commands/IEditorCommand.h": {
    summary: "定义编辑器命令的抽象接口 IEditorCommand，声明纯虚函数 Execute、Undo 和虚函数 TryMerge，是所有具体命令类的基类。",
    tags: ["interface", "command-pattern", "undo-redo", "polymorphism"],
    complexity: "simple"
  }
};

// Function/class summary & tag definitions
const subMeta = {
  // CameraController.h
  "class:CameraController": {
    summary: "编辑器摄像机控制器，作为 Native Script 组件响应 OnCreate/OnDestroy/OnUpdate 生命周期，处理键盘输入驱动摄像机移动。",
    tags: ["camera", "editor", "input-handling", "script-component"],
    complexity: "simple"
  },
  // EditorExternalFileDrop.cpp
  "function:Install": {
    summary: "安装 GLFW 文件拖放回调，将拖入窗口的外部文件路径存入内部队列。",
    tags: ["file-drop", "glfw", "callback"],
    complexity: "simple"
  },
  "function:ConsumePendingPaths": {
    summary: "消费并清空待处理的拖放文件路径队列，返回移动语义的路径列表。",
    tags: ["file-drop", "queue", "utility"],
    complexity: "simple"
  },
  // EditorExternalFileDrop.h
  "class:EditorExternalFileDrop": {
    summary: "编辑器外部文件拖放静态工具类，管理 GLFW drop callback 和拖放路径队列。",
    tags: ["file-drop", "static-class", "utility"],
    complexity: "simple"
  },
  // EditorLayer.cpp - functions
  "function:OnAttach": {
    summary: "编辑器层附加到应用时初始化，加载启动画面纹理并应用初始窗口尺寸。",
    tags: ["editor", "initialization", "splash-screen"],
    complexity: "moderate"
  },
  "function:ApplySplashWindowSize": {
    summary: "根据启动画面纹理尺寸调整窗口大小为合适比例，并居中显示。",
    tags: ["editor", "splash-screen", "window-management"],
    complexity: "simple"
  },
  "function:AdvanceEditorStartupLoading": {
    summary: "执行编辑器启动加载流程：加载图标纹理、字体、天空盒、创建帧缓冲和编辑器摄像机、安装文件拖放、加载最近项目。",
    tags: ["editor", "startup", "initialization", "resource-loading"],
    complexity: "complex"
  },
  "function:OnUpdate": {
    summary: "编辑器主更新循环：处理启动过渡、脚本编译状态、视口渲染（绑定帧缓冲、清除、2D/3D覆盖层绘制）、鼠标拾取和 Tilemap 编辑叠加层。",
    tags: ["editor", "update-loop", "viewport", "rendering", "tilemap"],
    complexity: "complex"
  },
  "function:OnImGuiRender": {
    summary: "编辑器 ImGui 渲染入口：绘制启动画面/项目中心、主停靠空间、菜单栏、统计面板、视口窗口（含 ImGuizmo 操纵器）、Tilemap 编辑交互和工具栏。",
    tags: ["editor", "imgui", "docking", "viewport", "guizmo", "tilemap"],
    complexity: "complex"
  },
  "function:OnEvent": {
    summary: "编辑器事件分发器，将 KeyPressed 和 MouseButtonPressed 事件路由到对应处理函数。",
    tags: ["editor", "event-dispatching", "input"],
    complexity: "simple"
  },
  "function:OnKeyPressed": {
    summary: "处理键盘快捷键：Ctrl+N 新建场景、Ctrl+O 打开场景、Ctrl+Shift+S 另存为、Ctrl+S 保存项目、Ctrl+D 复制实体、Delete 删除实体、Ctrl+Z/Ctrl+Y 撤销/重做、Gizmo 操作切换。",
    tags: ["editor", "keyboard-shortcuts", "input-handling"],
    complexity: "complex"
  },
  "function:OnMouseButtonPressed": {
    summary: "处理鼠标点击事件，支持 Tilemap 绘制模式下的实体选择和点击拾取。",
    tags: ["editor", "mouse-input", "tilemap", "entity-selection"],
    complexity: "simple"
  },
  "function:UpdateTilemapHoverFromInput": {
    summary: "根据鼠标在视口中的位置计算 Tilemap 悬停的 Tile 坐标，考虑摄像机投影和实体变换。",
    tags: ["tilemap", "hover", "coordinate-transform", "editor"],
    complexity: "moderate"
  },
  "function:OnOverlayRender": {
    summary: "渲染编辑器叠加层：2D/3D 网格、碰撞体可视化（Box/Circle/Tilemap）、精灵轮廓、选中实体高亮和 Tilemap 编辑叠加层。",
    tags: ["editor", "overlay", "rendering", "collision-visualization", "debug-draw"],
    complexity: "complex"
  },
  "function:CreateProject": {
    summary: "创建新项目：生成目录结构、项目文件、默认场景（含摄像机实体），根据 2D/3D 模式设置投影类型。",
    tags: ["editor", "project-creation", "scene-setup"],
    complexity: "complex"
  },
  "function:OpenProject": {
    summary: "打开已有项目：加载项目配置、同步 ScriptCore、处理最近项目列表、加载启动场景或创建新场景。",
    tags: ["editor", "project-management", "scene-loading"],
    complexity: "complex"
  },
  "function:DrawMainMenuBar": {
    summary: "绘制编辑器主菜单栏：文件菜单（保存/构建）、编辑菜单、场景菜单（新建/打开/另存为/关闭）、脚本菜单（编译/重新加载/打开 C# 项目）、视图菜单（切换面板可见性）。",
    tags: ["editor", "menu-bar", "imgui", "ui"],
    complexity: "moderate"
  },
  "function:BuildProject": {
    summary: "构建项目：收集引擎 DLL、项目 DLL、资产文件等依赖，复制到用户指定的输出目录完成打包。",
    tags: ["editor", "build", "packaging", "deployment"],
    complexity: "complex"
  },
  "function:OpenCSProject": {
    summary: "使用 ScriptIDELauncher 在外部 IDE（如 Visual Studio）中打开当前项目的 C# 脚本解决方案。",
    tags: ["editor", "scripting", "ide-integration"],
    complexity: "simple"
  },
  "function:NewScene": {
    summary: "创建新场景：重置选中实体、创建空白场景并设置天空盒和编辑器上下文。",
    tags: ["editor", "scene-management", "initialization"],
    complexity: "simple"
  },
  "function:LoadRecentProjects": {
    summary: "从 YAML 文件加载最近打开的项目列表，按时间戳排序并过滤已不存在的路径。",
    tags: ["editor", "project-management", "yaml", "recent-files"],
    complexity: "moderate"
  },
  "function:SaveRecentProjects": {
    summary: "将最近项目列表（最多 10 个）序列化为 YAML 格式并写入磁盘。",
    tags: ["editor", "project-management", "yaml", "serialization"],
    complexity: "simple"
  },
  "function:DrawStartupSplash": {
    summary: "绘制编辑器启动画面：全屏窗口显示引擎 Logo 纹理和带进度条的加载状态文本。",
    tags: ["editor", "splash-screen", "imgui", "startup"],
    complexity: "moderate"
  },
  "function:DrawProjectHub": {
    summary: "绘制项目中心界面：提供新建项目（名称、路径、2D/3D 模式选择）和打开已有项目（含最近项目列表）的功能。",
    tags: ["editor", "project-hub", "imgui", "ui"],
    complexity: "complex"
  },
  "function:OpenScene": {
    summary: "通过文件对话框选择并打开 .himii 场景文件，反序列化后设置为当前编辑上下文。",
    tags: ["editor", "scene-loading", "serialization", "file-dialog"],
    complexity: "simple"
  },
  "function:SaveScene": {
    summary: "保存当前场景到已有路径，若无路径则触发另存为流程。",
    tags: ["editor", "scene-saving", "serialization"],
    complexity: "simple"
  },
  "function:SaveProject": {
    summary: "保存项目：写入项目配置、场景、纹理元数据、TileMap 资产和动画资产，输出日志记录各步骤。",
    tags: ["editor", "project-saving", "serialization", "asset-management"],
    complexity: "moderate"
  },
  "function:SaveSceneAs": {
    summary: "通过文件对话框选择路径后将当前场景另存为新的 .himii 文件。",
    tags: ["editor", "scene-saving", "serialization", "file-dialog"],
    complexity: "simple"
  },
  "function:OnScenePlay": {
    summary: "处理场景播放按钮：若脚本正在编译则先请求编译，否则启动场景运行时。",
    tags: ["editor", "scene-playback", "scripting", "runtime"],
    complexity: "simple"
  },
  "function:StartScenePlay": {
    summary: "启动场景运行时模式：停止当前运行、清空控制台日志、复制场景并在副本上调用 OnRuntimeStart。",
    tags: ["editor", "scene-runtime", "simulation"],
    complexity: "moderate"
  },
  "function:OnSceneSimulate": {
    summary: "启动场景物理模拟模式：停止当前运行、复制场景并在副本上调用 OnSimulationStart。",
    tags: ["editor", "scene-simulation", "physics"],
    complexity: "simple"
  },
  "function:OnSceneStop": {
    summary: "停止场景运行/模拟：重置选中实体、调用 OnRuntimeStop/OnSimulationStop 并将编辑上下文切回编辑器场景。",
    tags: ["editor", "scene-lifecycle", "runtime"],
    complexity: "simple"
  },
  "function:OnDuplicateEntity": {
    summary: "复制当前选中的实体，使用场景的 DuplicateEntity 方法创建副本并选中。",
    tags: ["editor", "entity", "duplication"],
    complexity: "simple"
  },
  "function:RequestScriptCompile": {
    summary: "请求脚本编译：检查活动项目是否存在且未在编译中，停止场景运行后通过 ScriptEngine 发起编译。",
    tags: ["editor", "scripting", "compilation"],
    complexity: "simple"
  },
  "function:CompileAndReloadScripts": {
    summary: "强制重新编译并热重载脚本：停止场景运行、同步 ScriptCore 到项目目录后触发 ScriptEngine 编译。",
    tags: ["editor", "scripting", "compilation", "hot-reload"],
    complexity: "simple"
  },
  "function:UI_Toolbar": {
    summary: "绘制编辑器顶部工具栏：包含场景播放/停止/模拟按钮、Gizmo 操作模式切换（平移/旋转/缩放/通用）和工具图标。",
    tags: ["editor", "toolbar", "imgui", "guizmo", "play-controls"],
    complexity: "complex"
  },
  "function:UpdateTilemapPaintSession": {
    summary: "更新 Tilemap 绘制会话状态：检查选中实体是否有 TilemapComponent，验证资产有效性并管理绘制会话生命周期。",
    tags: ["tilemap", "editor", "painting", "session-management"],
    complexity: "moderate"
  },
  "function:IsTilemapEditContextActive": {
    summary: "检查 Tilemap 编辑上下文是否激活：验证选中实体、Tilemap 组件和资产管理器的有效性。",
    tags: ["tilemap", "editor", "context-validation"],
    complexity: "simple"
  },
  "function:TryGetTilemapPaintContext": {
    summary: "尝试获取 Tilemap 绘制上下文：返回实体引用、地图数据和变换组件的输出参数，失败时返回 false。",
    tags: ["tilemap", "editor", "context-resolution"],
    complexity: "simple"
  },
  "function:HandleTilemapScenePaint": {
    summary: "处理 Tilemap 场景绘制交互：响应键盘工具切换（铅笔/橡皮/填充/框选）、鼠标点击/拖拽绘制和框选应用。",
    tags: ["tilemap", "editor", "painting", "interaction"],
    complexity: "complex"
  },
  "function:DrawTilemapBoxSelectionOverlay": {
    summary: "绘制 Tilemap 框选覆盖层：显示框选区域边界和填充色，支持铅笔/橡皮/填充工具的视觉预览。",
    tags: ["tilemap", "editor", "overlay", "box-selection", "debug-draw"],
    complexity: "moderate"
  },
  "function:DrawTilemapGhostPreviewInViewport": {
    summary: "在视口中绘制 Tilemap 幽灵预览：根据当前选中 Tile 和工具模式在视口内显示半透明瓦片预览及 UV 贴图。",
    tags: ["tilemap", "editor", "preview", "ghost", "viewport"],
    complexity: "complex"
  },
  "function:DrawTilemapEditOverlay": {
    summary: "绘制 Tilemap 编辑叠加层：渲染编辑网格边界、悬停高亮、框选覆盖层和当前选中 Tile 的纹理预览。",
    tags: ["tilemap", "editor", "overlay", "debug-draw", "grid"],
    complexity: "complex"
  },
  // EditorLayoutDefaults.cpp
  "function:GetEditorLayoutIniPath": {
    summary: "获取编辑器布局 INI 文件的路径，位于可执行文件目录下的 imgui_layout.ini。",
    tags: ["editor", "layout", "file-path", "utility"],
    complexity: "simple"
  },
  "function:IniHasEditorDockLayout": {
    summary: "检查 INI 文件中是否已包含编辑器停靠布局配置（检测 DockSpace 等关键节点）。",
    tags: ["editor", "layout", "validation"],
    complexity: "simple"
  },
  "function:ShouldBuildDefaultLayout": {
    summary: "判断是否需要构建默认布局：INI 文件不存在或不含停靠布局信息时返回 true。",
    tags: ["editor", "layout", "decision"],
    complexity: "simple"
  },
  "function:DockLayoutHasSplitNodes": {
    summary: "检查指定 DockSpace ID 下是否已有分割节点，避免重复构建布局。",
    tags: ["editor", "layout", "docking", "validation"],
    complexity: "simple"
  },
  "function:ApplyDefaultDockLayoutIfNeeded": {
    summary: "按需构建编辑器默认停靠布局：将主视口分割为场景层级（左）、视口（中）、属性面板（右）、内容浏览器和日志面板（下）。",
    tags: ["editor", "layout", "docking", "initialization"],
    complexity: "moderate"
  },
  // TilemapEditorUtility.cpp
  "function:GetMapLocalOrigin": {
    summary: "计算 Tilemap 地图的本地原点世界坐标，基于编辑网格边界和 BottomLeft Tile 位置。",
    tags: ["tilemap", "coordinate-transform", "math"],
    complexity: "simple"
  },
  "function:LocalPositionToTileCoordinates": {
    summary: "将世界空间中的本地位置转换为 Tile 坐标，利用 TileLocalToCoordinates 和单元大小计算。",
    tags: ["tilemap", "coordinate-transform", "math"],
    complexity: "simple"
  },
  "function:ViewportMouseToWorld": {
    summary: "将视口鼠标坐标转换为世界坐标（Z=0 平面），委托给 ViewportMouseToWorldOnPlane。",
    tags: ["tilemap", "coordinate-transform", "viewport", "math"],
    complexity: "simple"
  },
  "function:ViewportMouseToWorldOnPlane": {
    summary: "将视口鼠标坐标通过逆视图投影矩阵和射线-平面求交转换为指定 Z 平面的世界坐标。",
    tags: ["tilemap", "coordinate-transform", "ray-plane-intersection", "math"],
    complexity: "moderate"
  },
  "function:WorldToViewportScreen": {
    summary: "将世界坐标映射到视口屏幕坐标，用于在视口中定位 UI 元素（幽灵预览、高亮等）。",
    tags: ["tilemap", "coordinate-transform", "viewport", "math"],
    complexity: "simple"
  },
  "function:AtlasCoordsToImGuiImageUVs": {
    summary: "将图集网格坐标转换为 ImGui Image 可用的 UV 坐标（左上-右下），通过 SpriteSheetUtility 辅助计算。",
    tags: ["tilemap", "atlas", "uv-mapping", "imgui"],
    complexity: "simple"
  },
  "function:AtlasCoordsToImGuiQuadUVs": {
    summary: "将图集网格坐标转换为 ImGui 四边形渲染所需的四角 UV 坐标（屏幕空间）。",
    tags: ["tilemap", "atlas", "uv-mapping", "imgui"],
    complexity: "simple"
  },
  // TilemapEditorUtility.h
  "class:TilemapEditorUtility": {
    summary: "Tilemap 编辑器数学工具类，封装视口坐标变换、Tile 坐标计算和图集 UV 映射的静态方法。",
    tags: ["tilemap", "utility", "math", "static-class"],
    complexity: "simple"
  },
  // EditorCommandHistory.cpp
  "function:EditorCommandHistory": {
    summary: "EditorCommandHistory 构造函数，设置撤销栈最大深度。",
    tags: ["undo-redo", "constructor", "editor"],
    complexity: "simple"
  },
  "function:Execute": {
    summary: "执行编辑器命令并将其推入撤销栈，同时清空重做栈，支持命令合并（TryMerge）和深度限制。",
    tags: ["undo-redo", "command-execution", "editor"],
    complexity: "moderate"
  },
  "function:Undo": {
    summary: "撤销最近执行的命令，将其从撤销栈移至重做栈。",
    tags: ["undo-redo", "command-pattern", "editor"],
    complexity: "simple"
  },
  "function:Redo": {
    summary: "重做最近撤销的命令，将其从重做栈移回撤销栈并重新执行。",
    tags: ["undo-redo", "command-pattern", "editor"],
    complexity: "simple"
  },
  "function:Clear": {
    summary: "清空撤销栈和重做栈中的所有命令。",
    tags: ["undo-redo", "cleanup", "editor"],
    complexity: "simple"
  },
  // EditorCommandHistory.h
  "class:EditorCommandHistory": {
    summary: "编辑器命令历史管理器类，维护 Undo/Redo 双栈和最大深度限制，支持 CanUndo/CanRedo 状态查询。",
    tags: ["undo-redo", "command-pattern", "history", "editor"],
    complexity: "simple"
  },
  // EditorCommands.cpp functions need disambiguation per class
  "function:CreateEntityCommand": {
    summary: "CreateEntityCommand 构造函数，接收场景引用和实体创建工厂函数，初始化快照为空。",
    tags: ["undo-redo", "entity", "command", "constructor"],
    complexity: "simple"
  },
  "function:DeleteEntityCommand": {
    summary: "DeleteEntityCommand 构造函数，保存场景引用、实体引用和实体恢复回调，创建实体快照。",
    tags: ["undo-redo", "entity", "command", "constructor"],
    complexity: "simple"
  },
  "function:ModifyTransformCommand": {
    summary: "ModifyTransformCommand 构造函数，记录实体标识符和变换前后的 TransformComponent 数据。",
    tags: ["undo-redo", "transform", "command", "constructor"],
    complexity: "simple"
  },
  "function:ModifyUITransformCommand": {
    summary: "ModifyUITransformCommand 构造函数，记录实体标识符和 UI 变换前后的 UITransformComponent 数据。",
    tags: ["undo-redo", "ui-transform", "command", "constructor"],
    complexity: "simple"
  },
  "function:ModifyEntityTagCommand": {
    summary: "ModifyEntityTagCommand 构造函数，记录实体标识符和标签修改前后的值。",
    tags: ["undo-redo", "entity-tag", "command", "constructor"],
    complexity: "simple"
  },
  "function:ApplyTransform": {
    summary: "将指定的变换数据应用到实体的 TransformComponent 或 UITransformComponent。",
    tags: ["undo-redo", "transform", "entity"],
    complexity: "simple"
  },
  "function:TryMerge": {
    summary: "尝试与相邻的同类型命令合并，通过更新最终状态来减少历史记录条目。",
    tags: ["undo-redo", "command-merging"],
    complexity: "simple"
  },
  "function:ApplyTag": {
    summary: "将指定的标签字符串应用到实体的 TagComponent。",
    tags: ["undo-redo", "entity-tag", "entity"],
    complexity: "simple"
  },
  // EditorCommands.h classes
  "class:CreateEntityCommand": {
    summary: "创建实体命令类，支持 Execute（创建/恢复实体）和 Undo（序列化快照后销毁），实现 IEditorCommand 接口。",
    tags: ["undo-redo", "entity", "command", "snapshot"],
    complexity: "simple"
  },
  "class:DeleteEntityCommand": {
    summary: "删除实体命令类，在构造时创建实体快照，Execute 销毁实体，Undo 从快照恢复。",
    tags: ["undo-redo", "entity", "command", "snapshot"],
    complexity: "simple"
  },
  "class:ModifyTransformCommand": {
    summary: "修改 Transform 命令类，记录变换前后状态，支持 Execute/Undo/TryMerge，实现连续变换操作的合并优化。",
    tags: ["undo-redo", "transform", "command", "merge"],
    complexity: "simple"
  },
  "class:ModifyUITransformCommand": {
    summary: "修改 UI Transform 命令类，管理 UITransformComponent 的撤销/重做和合并，与 ModifyTransformCommand 对称设计。",
    tags: ["undo-redo", "ui-transform", "command", "merge"],
    complexity: "simple"
  },
  "class:ModifyEntityTagCommand": {
    summary: "修改实体标签命令类，记录标签前后值，提供 Execute/Undo 实现标签的撤销修改。",
    tags: ["undo-redo", "entity-tag", "command"],
    complexity: "simple"
  },
  // EntitySnapshot.cpp
  "function:Serialize": {
    summary: "将实体通过 SceneSerializer 序列化为 YAML 字符串快照。",
    tags: ["serialization", "entity", "snapshot", "yaml"],
    complexity: "simple"
  },
  "function:Restore": {
    summary: "从 YAML 快照恢复实体：解析 YAML、检查 UUID 冲突、销毁现有实体后通过 SceneSerializer 反序列化创建。",
    tags: ["serialization", "entity", "snapshot", "yaml", "restore"],
    complexity: "simple"
  },
  // EntitySnapshot.h
  "class:EntitySnapshot": {
    summary: "实体快照工具类，提供静态方法 Serialize 和 Restore 用于实体的 YAML 序列化保存与恢复。",
    tags: ["serialization", "entity", "snapshot", "utility"],
    complexity: "simple"
  },
  // IEditorCommand.h
  "class:IEditorCommand": {
    summary: "编辑器命令抽象接口，定义 Execute/Undo 纯虚函数和 TryMerge 虚函数，作为命令模式的基础契约。",
    tags: ["interface", "command-pattern", "polymorphism", "editor"],
    complexity: "simple"
  }
};

// Disambiguation: for EditorCommands.cpp, functions with same name need file-specific keys
// Also for EditorCommandHistory.cpp
const fileSpecificMeta = {
  "HimiiEditor/src/commands/EditorCommandHistory.cpp": {
    "EditorCommandHistory": { summary: "EditorCommandHistory 构造函数，设置撤销栈最大深度。", tags: ["undo-redo", "constructor", "editor"], complexity: "simple" },
    "Execute": { summary: "执行编辑器命令并将其推入撤销栈，支持命令合并和深度限制。", tags: ["undo-redo", "command-execution", "editor"], complexity: "moderate" },
    "Undo": { summary: "撤销最近执行的命令，将其从撤销栈移至重做栈。", tags: ["undo-redo", "command-pattern", "editor"], complexity: "simple" },
    "Redo": { summary: "重做最近撤销的命令，将其从重做栈移回撤销栈并重新执行。", tags: ["undo-redo", "command-pattern", "editor"], complexity: "simple" },
    "Clear": { summary: "清空撤销栈和重做栈中的所有命令。", tags: ["undo-redo", "cleanup", "editor"], complexity: "simple" }
  },
  "HimiiEditor/src/commands/EditorCommands.cpp": {
    "CreateEntityCommand": { summary: "CreateEntityCommand 构造函数，接收场景引用和实体创建工厂函数。", tags: ["undo-redo", "entity", "command", "constructor"], complexity: "simple" },
    "DeleteEntityCommand": { summary: "DeleteEntityCommand 构造函数，保存场景引用和实体引用，创建实体快照。", tags: ["undo-redo", "entity", "command", "constructor"], complexity: "simple" },
    "ModifyTransformCommand": { summary: "ModifyTransformCommand 构造函数，记录实体标识符和变换前后数据。", tags: ["undo-redo", "transform", "command", "constructor"], complexity: "simple" },
    "ModifyUITransformCommand": { summary: "ModifyUITransformCommand 构造函数，记录实体标识符和 UI 变换前后数据。", tags: ["undo-redo", "ui-transform", "command", "constructor"], complexity: "simple" },
    "ModifyEntityTagCommand": { summary: "ModifyEntityTagCommand 构造函数，记录实体标识符和标签前后值。", tags: ["undo-redo", "entity-tag", "command", "constructor"], complexity: "simple" },
    "ApplyTransform": { summary: "将变换数据应用到实体（TransformComponent 或 UITransformComponent）。", tags: ["undo-redo", "transform", "entity"], complexity: "simple" },
    "Execute": { summary: "执行命令：应用 AfterTransform/AfterTag 或创建/删除实体。", tags: ["undo-redo", "command-execution", "editor"], complexity: "simple" },
    "Undo": { summary: "撤销命令：恢复 BeforeTransform/BeforeTag 或序列化后销毁/从快照恢复实体。", tags: ["undo-redo", "command-pattern", "editor"], complexity: "simple" },
    "TryMerge": { summary: "尝试与相邻同类型命令合并以压缩历史记录。", tags: ["undo-redo", "command-merging"], complexity: "simple" },
    "ApplyTag": { summary: "将标签字符串应用到实体的 TagComponent。", tags: ["undo-redo", "entity-tag", "entity"], complexity: "simple" }
  },
  "HimiiEditor/src/commands/EntitySnapshot.cpp": {
    "Serialize": { summary: "将实体序列化为 YAML 字符串快照。", tags: ["serialization", "entity", "snapshot", "yaml"], complexity: "simple" },
    "Restore": { summary: "从 YAML 快照恢复实体：解析、冲突检查、反序列化。", tags: ["serialization", "entity", "snapshot", "yaml", "restore"], complexity: "simple" }
  },
  "HimiiEditor/src/EditorLayoutDefaults.cpp": {
    "GetEditorLayoutIniPath": { summary: "获取编辑器布局 INI 文件的路径。", tags: ["editor", "layout", "file-path", "utility"], complexity: "simple" },
    "IniHasEditorDockLayout": { summary: "检查 INI 文件中是否已包含停靠布局配置。", tags: ["editor", "layout", "validation"], complexity: "simple" },
    "ShouldBuildDefaultLayout": { summary: "判断是否需要构建默认布局。", tags: ["editor", "layout", "decision"], complexity: "simple" },
    "DockLayoutHasSplitNodes": { summary: "检查 DockSpace 是否已有分割节点。", tags: ["editor", "layout", "docking", "validation"], complexity: "simple" },
    "ApplyDefaultDockLayoutIfNeeded": { summary: "按需构建编辑器默认停靠布局。", tags: ["editor", "layout", "docking", "initialization"], complexity: "moderate" }
  },
  "HimiiEditor/src/TilemapEditorUtility.cpp": {
    "GetMapLocalOrigin": { summary: "计算 Tilemap 地图的本地原点世界坐标。", tags: ["tilemap", "coordinate-transform", "math"], complexity: "simple" },
    "LocalPositionToTileCoordinates": { summary: "将世界空间本地位置转换为 Tile 坐标。", tags: ["tilemap", "coordinate-transform", "math"], complexity: "simple" },
    "ViewportMouseToWorld": { summary: "将视口鼠标坐标转换为世界坐标（Z=0 平面）。", tags: ["tilemap", "coordinate-transform", "viewport", "math"], complexity: "simple" },
    "ViewportMouseToWorldOnPlane": { summary: "将视口鼠标坐标通过逆投影矩阵转换为指定平面世界坐标。", tags: ["tilemap", "coordinate-transform", "ray-plane-intersection", "math"], complexity: "moderate" },
    "WorldToViewportScreen": { summary: "将世界坐标映射到视口屏幕坐标。", tags: ["tilemap", "coordinate-transform", "viewport", "math"], complexity: "simple" },
    "AtlasCoordsToImGuiImageUVs": { summary: "将图集网格坐标转换为 ImGui Image UV 坐标。", tags: ["tilemap", "atlas", "uv-mapping", "imgui"], complexity: "simple" },
    "AtlasCoordsToImGuiQuadUVs": { summary: "将图集网格坐标转换为 ImGui 四边形四角 UV 坐标。", tags: ["tilemap", "atlas", "uv-mapping", "imgui"], complexity: "simple" }
  }
};

function getFuncMeta(filePath, funcName) {
  // First check file-specific meta
  if (fileSpecificMeta[filePath] && fileSpecificMeta[filePath][funcName]) {
    return fileSpecificMeta[filePath][funcName];
  }
  // Then check global
  const key = `function:${funcName}`;
  if (subMeta[key]) return subMeta[key];
  return { summary: `成员函数 ${funcName} 的实现。`, tags: ["method"], complexity: "simple" };
}

function getClassMeta(className) {
  const key = `class:${className}`;
  if (subMeta[key]) return subMeta[key];
  return { summary: `类 ${className} 的定义。`, tags: ["class"], complexity: "simple" };
}

// Build all nodes and edges
let allNodes = [];
let allEdges = [];

// Helper: file node
function makeFileNode(path) {
  const name = path.split('/').pop();
  const meta = fileMeta[path] || { summary: "C++ source file.", tags: ["source"], complexity: "simple" };
  return {
    id: `file:${path}`,
    type: "file",
    name,
    filePath: path,
    summary: meta.summary,
    tags: meta.tags,
    complexity: meta.complexity,
    ...(meta.languageNotes ? { languageNotes: meta.languageNotes } : {})
  };
}

// Disambiguation rules for functions with duplicate names in the same file
// EditorCommands.cpp: methods are grouped by class (constructor/Execute/Undo/TryMerge pattern)
const commandsCppClassOrder = [
  { name: "CreateEntityCommand", endLine: 33 },
  { name: "DeleteEntityCommand", endLine: 56 },
  { name: "ModifyTransformCommand", endLine: 93 },
  { name: "ModifyUITransformCommand", endLine: 130 },
  { name: "ModifyEntityTagCommand", endLine: 156 }
];

function getQualifiedName(filePath, func) {
  // EditorCommands.cpp disambiguation
  if (filePath === "HimiiEditor/src/commands/EditorCommands.cpp") {
    for (const cls of commandsCppClassOrder) {
      if (func.startLine <= cls.endLine) {
        return `${cls.name}::${func.name}`;
      }
    }
  }
  // EditorLayer.cpp OpenScene overloads - differentiate by line
  if (filePath === "HimiiEditor/src/EditorLayer.cpp" && func.name === "OpenScene") {
    if (func.params && func.params.length > 0) {
      return `OpenScene(path)`;  // overload with path param
    }
    return `OpenScene()`;  // no-arg overload
  }
  return func.name;
}

// Helper: function node
function makeFuncNode(filePath, func) {
  const qualifiedName = getQualifiedName(filePath, func);
  const meta = getFuncMeta(filePath, func.name);
  return {
    id: `function:${filePath}:${qualifiedName}`,
    type: "function",
    name: qualifiedName,
    filePath,
    lineRange: [func.startLine, func.endLine],
    summary: meta.summary,
    tags: meta.tags,
    complexity: meta.complexity
  };
}

// Helper: class node
function makeClassNode(filePath, cls) {
  const meta = getClassMeta(cls.name);
  return {
    id: `class:${filePath}:${cls.name}`,
    type: "class",
    name: cls.name,
    filePath,
    lineRange: [cls.startLine, cls.endLine],
    summary: meta.summary,
    tags: meta.tags,
    complexity: meta.complexity
  };
}

// Process each file
results.results.forEach(r => {
  const path = r.path;

  // File node
  allNodes.push(makeFileNode(path));
  const fileNodeId = `file:${path}`;

  // Import edges
  const importTargets = batchImportData[path] || [];
  importTargets.forEach(target => {
    allEdges.push({
      source: fileNodeId,
      target: `file:${target}`,
      type: "imports",
      direction: "forward",
      weight: 0.7
    });
  });

  // Functions
  (r.functions || []).forEach(func => {
    const len = func.endLine - func.startLine + 1;
    const exportNames = new Set((r.exports || []).map(e => e.name));
    const isExported = exportNames.has(func.name);

    // Include if 10+ lines or exported
    if (len >= 10 || isExported) {
      const fnode = makeFuncNode(path, func);
      allNodes.push(fnode);

      // contains edge
      allEdges.push({
        source: fileNodeId,
        target: fnode.id,
        type: "contains",
        direction: "forward",
        weight: 1.0
      });

      // exports edge if exported
      if (isExported) {
        allEdges.push({
          source: fileNodeId,
          target: fnode.id,
          type: "exports",
          direction: "forward",
          weight: 0.8
        });
      }
    }
  });

  // Classes
  (r.classes || []).forEach(cls => {
    const len = cls.endLine - cls.startLine + 1;
    const exportNames = new Set((r.exports || []).map(e => e.name));
    const isExported = exportNames.has(cls.name);

    // Skip forward declarations
    if (cls.name === 'GLFWwindow') return;

    // Include if 2+ methods or 20+ lines or exported
    if (cls.methods.length >= 2 || len >= 20 || isExported) {
      const cnode = makeClassNode(path, cls);
      allNodes.push(cnode);

      // contains edge
      allEdges.push({
        source: fileNodeId,
        target: cnode.id,
        type: "contains",
        direction: "forward",
        weight: 1.0
      });

      // exports edge if exported
      if (isExported) {
        allEdges.push({
          source: fileNodeId,
          target: cnode.id,
          type: "exports",
          direction: "forward",
          weight: 0.8
        });
      }
    }
  });
});

// Cross-file calls based on call graph analysis
const crossFileCalls = [
  // EditorLayer.cpp -> EditorExternalFileDrop.cpp
  ["function:HimiiEditor/src/EditorLayer.cpp:AdvanceEditorStartupLoading", "function:HimiiEditor/src/EditorExternalFileDrop.cpp:Install"],
  // EditorLayer.cpp -> EditorLayoutDefaults.cpp
  ["function:HimiiEditor/src/EditorLayer.cpp:OnImGuiRender", "function:HimiiEditor/src/EditorLayoutDefaults.cpp:ApplyDefaultDockLayoutIfNeeded"],
  // EditorLayer.cpp -> TilemapEditorUtility.cpp
  ["function:HimiiEditor/src/EditorLayer.cpp:UpdateTilemapHoverFromInput", "function:HimiiEditor/src/TilemapEditorUtility.cpp:ViewportMouseToWorldOnPlane"],
  ["function:HimiiEditor/src/EditorLayer.cpp:UpdateTilemapHoverFromInput", "function:HimiiEditor/src/TilemapEditorUtility.cpp:LocalPositionToTileCoordinates"],
  ["function:HimiiEditor/src/EditorLayer.cpp:HandleTilemapScenePaint", "function:HimiiEditor/src/TilemapEditorUtility.cpp:ViewportMouseToWorldOnPlane"],
  ["function:HimiiEditor/src/EditorLayer.cpp:HandleTilemapScenePaint", "function:HimiiEditor/src/TilemapEditorUtility.cpp:LocalPositionToTileCoordinates"],
  ["function:HimiiEditor/src/EditorLayer.cpp:DrawTilemapGhostPreviewInViewport", "function:HimiiEditor/src/TilemapEditorUtility.cpp:WorldToViewportScreen"],
];

crossFileCalls.forEach(([src, tgt]) => {
  allEdges.push({
    source: src,
    target: tgt,
    type: "calls",
    direction: "forward",
    weight: 0.8
  });
});

// Implements: EditorCommands -> IEditorCommand
const commandClasses = [
  "CreateEntityCommand", "DeleteEntityCommand", "ModifyTransformCommand",
  "ModifyUITransformCommand", "ModifyEntityTagCommand"
];
commandClasses.forEach(clsName => {
  allEdges.push({
    source: `class:HimiiEditor/src/commands/EditorCommands.h:${clsName}`,
    target: "class:HimiiEditor/src/commands/IEditorCommand.h:IEditorCommand",
    type: "implements",
    direction: "forward",
    weight: 0.9
  });
});

// depends_on: EditorCommandHistory.cpp -> IEditorCommand.h
allEdges.push({
  source: "file:HimiiEditor/src/commands/EditorCommandHistory.cpp",
  target: "class:HimiiEditor/src/commands/IEditorCommand.h:IEditorCommand",
  type: "depends_on",
  direction: "forward",
  weight: 0.6
});

// depends_on: EditorCommands.cpp -> EntitySnapshot
allEdges.push({
  source: "file:HimiiEditor/src/commands/EditorCommands.cpp",
  target: "class:HimiiEditor/src/commands/EntitySnapshot.h:EntitySnapshot",
  type: "depends_on",
  direction: "forward",
  weight: 0.6
});

// depends_on: EditorLayer.cpp -> CameraController, EditorCommands, EditorCommandHistory
allEdges.push({
  source: "file:HimiiEditor/src/EditorLayer.cpp",
  target: "class:HimiiEditor/src/CamerController.h:CameraController",
  type: "depends_on",
  direction: "forward",
  weight: 0.6
});
allEdges.push({
  source: "file:HimiiEditor/src/EditorLayer.cpp",
  target: "class:HimiiEditor/src/commands/EditorCommands.h:ModifyTransformCommand",
  type: "depends_on",
  direction: "forward",
  weight: 0.6
});
allEdges.push({
  source: "file:HimiiEditor/src/EditorLayer.cpp",
  target: "class:HimiiEditor/src/commands/EditorCommands.h:ModifyUITransformCommand",
  type: "depends_on",
  direction: "forward",
  weight: 0.6
});
allEdges.push({
  source: "file:HimiiEditor/src/EditorLayer.cpp",
  target: "class:HimiiEditor/src/commands/EditorCommands.h:DeleteEntityCommand",
  type: "depends_on",
  direction: "forward",
  weight: 0.6
});
allEdges.push({
  source: "file:HimiiEditor/src/EditorLayer.cpp",
  target: "class:HimiiEditor/src/commands/EditorCommandHistory.h:EditorCommandHistory",
  type: "depends_on",
  direction: "forward",
  weight: 0.6
});

console.log('Total nodes:', allNodes.length);
console.log('Total edges:', allEdges.length);

// Split into 2 parts (sorted alphabetically, 8 + 7 files)
const allFiles = results.results.map(r => r.path).sort();
const partSize = Math.ceil(allFiles.length / 2); // 8
const part1Files = new Set(allFiles.slice(0, partSize));
const part2Files = new Set(allFiles.slice(partSize));

console.log('Part 1 files:', [...part1Files]);
console.log('Part 2 files:', [...part2Files]);

function splitNodesAndEdges(nodes, edges, fileSet) {
  const partNodes = nodes.filter(n => {
    if (n.filePath) return fileSet.has(n.filePath);
    // For nodes without filePath, try to extract from id
    const id = n.id;
    for (let fp of fileSet) {
      if (id.includes(fp)) return true;
    }
    return false;
  });

  const partNodeIds = new Set(partNodes.map(n => n.id));
  const partEdges = edges.filter(e => partNodeIds.has(e.source));

  return { nodes: partNodes, edges: partEdges };
}

const part1 = splitNodesAndEdges(allNodes, allEdges, part1Files);
const part2 = splitNodesAndEdges(allNodes, allEdges, part2Files);

console.log('Part 1 nodes:', part1.nodes.length, 'edges:', part1.edges.length);
console.log('Part 2 nodes:', part2.nodes.length, 'edges:', part2.edges.length);

// Write output files
const outDir = 'D:/Himii-Engine/.understand-anything/intermediate';
if (!fs.existsSync(outDir)) fs.mkdirSync(outDir, { recursive: true });

fs.writeFileSync(`${outDir}/batch-3-part-1.json`, JSON.stringify(part1, null, 2));
fs.writeFileSync(`${outDir}/batch-3-part-2.json`, JSON.stringify(part2, null, 2));

console.log('Wrote batch-3-part-1.json and batch-3-part-2.json');

// Verify import edge count
let expectedImportEdges = 0;
Object.values(batchImportData).forEach(arr => { expectedImportEdges += arr.length; });
let actualImportEdges = allEdges.filter(e => e.type === 'imports').length;
console.log('Expected import edges:', expectedImportEdges, 'Actual:', actualImportEdges, 'Match:', expectedImportEdges === actualImportEdges);

// Verify no duplicate node IDs
const nodeIds = allNodes.map(n => n.id);
const uniqueIds = new Set(nodeIds);
if (nodeIds.length !== uniqueIds.size) {
  console.log('WARNING: Duplicate node IDs found!');
  const dupes = nodeIds.filter((id, i) => nodeIds.indexOf(id) !== i);
  console.log('Duplicates:', [...new Set(dupes)]);
} else {
  console.log('All node IDs unique: OK');
}

// Verify no self-referencing edges
const selfEdges = allEdges.filter(e => e.source === e.target);
if (selfEdges.length > 0) {
  console.log('WARNING: Self-referencing edges:', selfEdges.length);
} else {
  console.log('No self-referencing edges: OK');
}
