# HimiiEngine

[![编程语言](https://img.shields.io/badge/编程语言-C++_17-blue.svg?style=for-the-badge)](#)
[![构建工具](https://img.shields.io/badge/构建工具-CMake_>=3.12-green.svg?style=for-the-badge)](#)

面向 2D 游戏开发的集成引擎：C++ 运行时、可视化编辑器（HimiiEditor）与 C# 脚本系统（.NET 8 + CoreCLR）。

**完整文档**：[Docs/docs/index.md](Docs/docs/index.md)（快速开始、用户手册、教程、脚本 API、开发者手册）

> [!IMPORTANT]
> 本项目遵循 [**MIT License**](https://github.com/HimiiFish/Himii-Engine/blob/main/LICENSE)

## 快速构建

克隆仓库后（建议 `git clone --recursive`），生成本地 CMake 预设并构建编辑器：

```powershell
# Windows
copy CMakePresets.json.example CMakePresets.json
python build.py debug
```

```bash
# Linux
cp CMakePresets.json.example CMakePresets.json
python build.py debug
```

本地 `CMakePresets.json` 不会提交到 Git。环境要求、手动 CMake 命令、HimiiRuntime 与故障排除见 **[源码构建文档](Docs/docs/DevManual/BuildingFromSource.md)**。

## 主要目标

| 目标 | Debug 输出路径（示例） |
|------|------------------------|
| **HimiiEditor**（编辑器） | `build/x64-debug/bin/HimiiEditor/Debug/HimiiEditor.exe` |
| **HimiiRuntime**（运行时） | `build/x64-debug/bin/HimiiRuntime/Debug/HimiiRuntime.exe` |

构建 HimiiEditor 时会自动编译并拷贝 **ScriptCore.dll**。在 Visual Studio 中将启动项目设为 **HimiiEditor** 即可调试。
