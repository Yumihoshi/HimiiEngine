# 开发路线图

本文档采用 **核心模块 - 最小执行单元** 的结构对应当下的开发进度与未来的规划。

---

## 📅 已完成特性 (Phase 1: Foundation)

### 核心模块：基础架构 (Core Architecture)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 说明 |
| :--- | :--- | :--- |
| [x] | **Application Loop** | 实现应用启动、主循环、关闭流程 |
| [x] | **LayerStack** | 实现层的压栈、弹栈与遍历更新 |
| [x] | **Window Abstraction** | 基于 GLFW 封装跨平台窗口创建与上下文管理 |
| [x] | **Event System** | 实现键盘、鼠标、窗口事件的分发与回调 |
| [x] | **Log System** | 集成 organic spdlog 日志库 |

### 核心模块：渲染系统 (Renderer)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 说明 |
| :--- | :--- | :--- |
| [x] | **OpenGL Context** | 集成 GLAD 并管理 OpenGL 上下文 |
| [x] | **Shader Class** | 着色器的加载、编译与链接 |
| [x] | **Buffer System** | VertexBuffer, IndexBuffer, VertexArray 的抽象与实现 |
| [x] | **Texture System** | 2D 纹理加载与绑定 (stb_image) |
| [x] | **Orthographic Camera** | 正交相机控制器 |
| [x] | **Batch Renderer** | 2D 批处理渲染 (自动合并 DrawCall) |
| [x] | **Framebuffer** | 离屏渲染支持 (仅颜色附件) |
| [x] | **SubTexture2D** | 纹理图集切片支持 |

### 核心模块：物理系统 (Physics 2D)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 说明 |
| :--- | :--- | :--- |
| [x] | **Box2D World** | 物理世界初始化与步进 (Step) |
| [x] | **Rigidbody2D** | 刚体组件属性同步 (Transform <-> Box2D Body) |
| [x] | **BoxCollider2D** | 矩形碰撞体与夹具 (Fixture) 创建 |
| [x] | **Collision Events 2D** | `OnCollisionEnter2D` / `OnCollisionExit2D` 脚本回调 |

### 核心模块：脚本系统 (Scripting)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 说明 |
| :--- | :--- | :--- |
| [x] | **CoreCLR Host** | .NET 8 + 可收集 AssemblyLoadContext（非 Mono） |
| [x] | **Script Class** | C# 类加载与实例化（OnCreate / OnUpdate / OnDestroy） |
| [x] | **Internal Calls** | C++ 与 C# 互操作绑定 |
| [x] | **Input API** | C# `Input.IsKeyDown` 等接口 |
| [x] | **Script Fields** | Inspector 公开字段读写与 YAML 序列化（含 Entity 引用） |
| [x] | **Async Compile** | `dotnet build` 异步编译与文件监视 |
| [x] | **Script Console** | 编译日志面板，错误行跳转 IDE |
| [x] | **IDE Launcher** | 全局/项目 IDE 配置（VS / VS Code / Rider / Custom） |
| [x] | **Script Log API** | C# `Log.Info` / `Warning` / `Error`，经 C++ `Log::PrintMessage` 输出 |

### 核心模块：编辑器 (Editor Interface)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 说明 |
| :--- | :--- | :--- |
| [x] | **DockSpace** | 全屏停靠布局支持 |
| [x] | **Scene Viewport** | 场景编辑视口 (渲染 FBO 纹理) |
| [x] | **Hierarchy Panel** | 实体层级树状列表 |
| [x] | **Properties Panel** | 组件属性检视与修改 |
| [x] | **Content Browser** | 资源文件浏览与拖拽 |
| [x] | **ImGuizmo** | 视口内物体变换 Gizmo |
| [x] | **Editor Preferences** | 全局编辑器设置（含默认脚本 IDE） |
| [x] | **Project Settings** | 项目级 IDE 覆盖与 `.hproj` 序列化 |
| [x] | **Console Panel** | 运行时脚本日志 ImGui 面板（`Window → Console`） |

### 核心模块：发布 (Distribution)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 说明 |
| :--- | :--- | :--- |
| [x] | **Runtime Application** | `HimiiRuntime` 剥离编辑器的游戏运行程序 |

---

## 🚀 未来开发计划 (Phase 2 & Beyond)

### 核心模块：渲染增强 (Advanced Rendering)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 预期内容 |
| :--- | :--- | :--- |
| [ ] | **Directional Shadow** | 级联阴影贴图 (CSM) |
| [ ] | **Point Shadow** | 全向阴影 (CubeMap) |
| [ ] | **Bloom** | 泛光后处理特效 |
| [ ] | **Post-Processing Stack** | 包含 ToneMapping, Gamma Correction 的后处理管线 |
| [ ] | **Particle System** | 基于 GPU 或 CPU 的粒子发射器（编辑器部分已有基础） |

### 核心模块：物理系统升级 (Physics 3D)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 预期内容 |
| :--- | :--- | :--- |
| [ ] | **Physics Engine Integration** | 集成 Jolt Physics 或 PhysX |
| [ ] | **Sphere Collider** | 球体碰撞组件 |
| [ ] | **Capsule Collider** | 胶囊体碰撞组件 |
| [ ] | **Mesh Collider** | 凸包或三角网格碰撞支持 |
| [ ] | **Physics Debug Draw** | 线框可视化调试 |

### 核心模块：游戏性与脚本 (Gameplay & Scripting)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 预期内容 |
| :--- | :--- | :--- |
| [ ] | **Prefab System** | 预制体资产的保存与实例化 |
| [ ] | **Scene Transition** | 运行时场景切换 API |
| [ ] | **[SerializeField]** | 支持 private 字段在 Inspector 中显示 |

### 核心模块：编辑器交互 (Editor UX)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 预期内容 |
| :--- | :--- | :--- |
| [ ] | **Undo/Redo** | 命令模式实现操作撤销重做 |
| [ ] | **Asset Importer** | 统一的资源导入设置面板 |
| [ ] | **Console 增强** | 日志过滤/搜索/复制；默认展示引擎日志；`Console.WriteLine` 重定向等 |

### 核心模块：发布 (Distribution)
| 状态 | 最小执行单元 (Minimum Execution Unit) | 预期内容 |
| :--- | :--- | :--- |
| [ ] | **Project Packager** | 自动打包脚本与资源到发布的目录 |
| [ ] | **Build Pipeline** | 确保 GameAssembly、ScriptCore、资源、`.hproj` 一并输出 |
