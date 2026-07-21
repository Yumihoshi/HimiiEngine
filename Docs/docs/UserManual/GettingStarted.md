# 快速开始 (Getting Started)

欢迎使用 HimiiEngine。本指南帮助你在本地构建引擎并创建第一个项目。

## 1. 获取引擎（源码构建）

当前请从源码构建 HimiiEngine。完整步骤见开发者手册中的 [源码构建](../DevManual/BuildingFromSource.md)。

简要前置要求：

- Windows 10/11（Linux 见仓库 README）
- Visual Studio 2022（「使用 C++ 的桌面开发」工作负载）
- CMake 3.12+
- .NET 8 SDK
- Git（建议 `git clone --recursive` 以包含 vcpkg 子模块）

构建编辑器（Debug 示例）：

```powershell
copy CMakePresets.json.example CMakePresets.json

cmake --preset x64-debug
cmake --build --preset build-x64-debug-win -j 8
```

也可使用 `python build.py debug` 或 `build_py.bat`。

## 2. 启动编辑器

构建完成后，运行 HimiiEditor 可执行文件，例如：

`build/x64-debug/bin/HimiiEditor/Debug/HimiiEditor.exe`

也可在 Visual Studio 中将 **HimiiEditor** 设为启动项目并调试运行。

> **【配图占位】** `UserManual/images/editor-hub.png` — 启动后 Hub（New Project / Open Project / Recent）

说明：编辑器界面与菜单中的名称为 **HimiiEditor**；Release 配置下生成的可执行文件名为 `HimiiEngine.exe`，路径为 `build/x64-release/bin/HimiiEditor/Release/HimiiEngine.exe`。

首次运行前需成功构建 **ScriptCore**（构建 HimiiEditor 时通常会通过 `ScriptCore_Build` 目标自动完成）。

## 3. 创建新项目

1. 启动 HimiiEditor，在 Hub 点击 **Create New Project**（或菜单 **File → New Project**）。
2. 输入 **项目名称** 与 **保存路径**。
3. 选择项目模板：**2D Project**（正交相机）或 **3D Project**（透视相机）。
4. 点击 **Create** 确认。

> **【配图占位】** `UserManual/images/new-project-dialog.png` — New Project 弹窗（含 2D / 3D 单选）

创建成功后会自动生成 `assets/` 目录结构、`GameAssembly.csproj`、默认场景与 `.hproj` 项目文件。

## 4. 在编辑器中运行场景

1. 在 **Content Browser** 中进入项目的 `assets/scenes` 目录。
2. 双击要编辑的 `.himii` 场景文件。
3. 点击工具栏上的 **Play**（绿色三角形）在编辑器内运行当前场景。

## 5. 下一步：跟着教程做平台跳跃

推荐直接完成 **[教程：2D 平台跳跃](Tutorial2DPlatformer.md)**（从地面、动画到脚本一步走完）。

之后可按需查阅：

- [编辑器与功能](EditorFeatures.md) — 动画、物理、脚本等总览  
- [脚本 API](ScriptingAPI.md) — API 参考  
