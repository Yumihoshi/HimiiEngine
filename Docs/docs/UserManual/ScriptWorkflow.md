# 脚本工作流 (Script Workflow)

本文说明在 HimiiEditor 中编写、编译与调试 C# 游戏脚本的典型流程。

## 项目中的脚本文件

打开项目后，脚本通常位于：

```
<ProjectDir>/
├── assets/
│   └── scripts/          # 游戏 C# 源码（*.cs）
├── <ProjectName>.sln     # 供 IDE 打开
├── <ProjectName>.csproj
└── <ProjectName>.hproj   # 项目配置（含 ScriptModulePath 等）
```

编译产物 **`GameAssembly.dll`**（及 `.pdb`）路径由 `.hproj` 的 `ScriptModulePath` 指定，构建时输出到项目目录或 `bin` 子目录（以项目模板为准）。

引擎自带 **`ScriptCore.dll`**（API：`Entity`、`Input`、`Log` 等），由 HimiiEditor 构建时拷贝到可执行文件旁，**不要**修改其命名空间下的核心类型。

## 编译

- 保存 `.cs` 后，编辑器 **文件监视** 可触发 **异步 `dotnet build`**（debounce）。
- 点击 **Play** 前若脚本有改动，会先编译再进入 Play；编译进行中若已在 Play，会先 Stop。
- 编译日志在 **Window → Script Console**：
  - 成功 / 失败状态
  - 点击 `path\file.cs(line,col): error` 行，在配置的 IDE 中打开对应位置

## 运行与热重载策略

- **Play**：复制场景、加载 `GameAssembly`、为带 `ScriptComponent` 的实体调用 `OnCreate`，每帧 `OnUpdate`。
- **Stop** 后回到编辑场景；修改脚本后需再次 **Play** 才会加载新 DLL（**不在 Play 中热替换程序集**）。
- 编译成功且曾中断 Play 时，可能提示重新按 Play。

运行时日志使用 `Log.Info` / `Warning` / `Error`，在 **Window → Console** 查看（见 [脚本 API](ScriptingAPI.md)）。

## IDE 配置

### 全局默认（Edit → Preferences）

- 选择 **Visual Studio**、**VS Code**、**Rider** 或 **Custom**（自定义可执行文件与参数模板）。
- 保存到引擎数据目录下的 `editor_settings.yaml`。

### 项目覆盖（File → Project Settings）

- **Use Editor Default**：继承全局设置。
- 或为本项目单独指定 IDE / Custom 路径。
- 写入 `.hproj`，随项目一并分享。

### 打开项目 / 脚本

- **File → Open C# Project**：用当前有效配置打开 `.sln` 或项目目录。
- 属性面板 **Script Component → Edit**：打开对应 `.cs` 文件。

**Custom** 参数占位符：`{ProjectDir}`、`{Solution}`、`{File}`、`{ProjectName}`。

## 常见问题

| 现象 | 处理 |
|------|------|
| Play 后脚本不执行 | 检查类名是否与 `ScriptComponent` 中一致（含命名空间）；Script Console 是否编译成功 |
| Inspector 无字段 | 字段须为 **public 实例** 字段，或 **private + `[SerializeField]`**；保存场景后重开验证 |
| `[SerializeField]` 无智能提示 | 用 IDE 打开 **`.sln`**；确认存在 `assets/scripts/Himii/SerializeField.cs`（打开项目或编译脚本时会自动同步）；项目根目录还需 `ScriptCore.dll` / `ScriptCore.xml`；重载 C# 语言服务 |
| Log 无输出 | 使用 `Himii.Log`，非 `Console.WriteLine`；确认 Console 面板已打开 |
| IDE 打不开 | Preferences / Project Settings 中检查 IDE 路径；VS Code 需 `code` 在 PATH |

## 相关文档

- [脚本 API](ScriptingAPI.md)
- [编辑器界面](EditorInterface.md)
- [源码构建](../DevManual/BuildingFromSource.md)
