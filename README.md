# HimiiEngine

[![编程语言](https://img.shields.io/badge/编程语言-C++_17-blue.svg?style=for-the-badge)](#)
[![构建工具](https://img.shields.io/badge/构建工具-Cmake_>=3.8-green.svg?style=for-the-badge)](#)

It is a modern game engine designed to build high-performance rendering, multi-threaded operation, and based on the ECS framework.

**中文文档**：[Docs/docs/index.md](Docs/docs/index.md)（用户手册、开发者手册、路线图）

> [!IMPORTANT]
> 本项目遵循 [**MIT License**](https://github.com/HimiiFish/HimiiEngine/blob/main/LICENSE)

## HimiiEngine 构建指南

本项目提供了跨平台的Python构建脚本 `build.py`，支持Windows和Linux平台。

## 前置要求

### Windows
- CMake 3.12 或更高版本
- Visual Studio 2022 (或更新版本)
- Python 3.6 或更高版本
- vcpkg (已包含在项目中)

### Linux
- CMake 3.12 或更高版本
- Ninja 构建系统
- GCC 或 Clang 编译器 (支持C++17)
- Python 3.6 或更高版本
- vcpkg (已包含在项目中)
- 安装 autoconf automake libtool libltdl-dev libgl1-mesa-dev libglu1-mesa-dev

## 使用方法

### 首次准备

克隆仓库后，先从模板生成本地 CMake 预设（只需一次）：

```powershell
# Windows
copy CMakePresets.json.example CMakePresets.json

# Linux
cp CMakePresets.json.example CMakePresets.json
```

本地 `CMakePresets.json` 不会提交到 Git。若需固定生成器，可在该文件中自行添加 `"generator": "Visual Studio 17 2022"` 或 `"generator": "Ninja"`。详见 [源码构建文档](Docs/docs/DevManual/BuildingFromSource.md)。

### 基本构建命令

```bash
# Debug 构建
python build.py debug

# Release 构建
python build.py release
```

### 高级选项

```bash
# 清理构建目录后重新构建
python build.py debug --clean

# 构建完成后自动运行程序
python build.py release --run

# 同时使用清理和自动运行
python build.py debug --clean --run

# 仅清理构建目录（不构建）
python build.py debug --clean-only
```

### 命令行参数说明

- `build_type`: 构建类型，可选 `debug` 或 `release`
- `--clean, -c`: 构建前清理构建目录
- `--run, -r`: 构建完成后自动运行生成的程序
- `--clean-only`: 仅清理构建目录，不进行构建
- `--help, -h`: 显示帮助信息

## 构建输出

构建目录（由本地 `CMakePresets.json` 与 `himii_set_output_dirs` 决定）：

- Windows Debug: `build/x64-debug/`
- Windows Release: `build/x64-release/`
- Linux: `build/linux-debug/` 或 `build/linux-release/`

主要可执行文件：

| 目标 | Windows Debug 路径（示例） |
|------|---------------------------|
| **HimiiEditor**（编辑器） | `build/x64-debug/bin/HimiiEditor/Debug/HimiiEditor.exe` |
| **HimiiRuntime**（运行时） | `build/x64-debug/bin/HimiiRuntime/Debug/HimiiRuntime.exe` |

构建 **HimiiEditor** 时会自动编译并拷贝 **ScriptCore**（`ScriptCore.dll`）到输出目录。将 Visual Studio 启动项目设为 **HimiiEditor** 即可调试编辑器。

## 故障排除

### 常见问题

1. **CMake未找到**
   - 确保CMake已安装并添加到系统PATH中

2. **Visual Studio未找到** (Windows)
   - 确保安装了Visual Studio 2022或更新版本
   - 确保安装了C++开发工具

3. **Ninja未找到** (Linux)
   - Ubuntu/Debian: `sudo apt install ninja-build`
   - CentOS/RHEL: `sudo yum install ninja-build`
   - Arch: `sudo pacman -S ninja`

4. **vcpkg依赖问题**
   - 确保vcpkg目录存在于项目根目录
   - 检查vcpkg是否已正确初始化

### 手动构建（备选方案）

如果Python脚本出现问题，可以使用传统的CMake命令：

#### Windows
```bash
# 首次：copy CMakePresets.json.example CMakePresets.json

# 配置
cmake --preset x64-debug

# 构建
cmake --build --preset build-x64-debug-win
```

#### Linux
```bash
# 首次：cp CMakePresets.json.example CMakePresets.json

# 配置
cmake --preset linux-debug

# 构建
cmake --build --preset build-linux-debug

