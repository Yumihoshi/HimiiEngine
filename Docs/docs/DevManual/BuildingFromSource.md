# 源码构建 (Building From Source)

## 前置要求 (Prerequisites)

* **操作系统**: Windows 10/11（Linux 构建说明见仓库 [README](https://github.com/HimiiFish/Himii-Engine/blob/main/README.md)）
* **Visual Studio 2022**: 安装 “使用 C++ 的桌面开发” 工作负载
* **CMake** 3.12+
* **Python 3**: 用于 `build.py`（可选）
* **.NET 8 SDK**: 编译 `ScriptCore` 与游戏脚本
* **Git**: 克隆时建议 `--recursive`（含 vcpkg 等子模块）

## 构建步骤（推荐）

### 方式一：CMake Preset

```powershell
git clone --recursive <repository-url>
cd Himii-Engine

cmake --preset x64-debug
cmake --build build/x64-debug --target HimiiEditor -j 8
```

Release 构建示例：

```powershell
cmake --preset x64-release
cmake --build --preset build-x64-release-win --config Release
```

### 方式二：Python 脚本

```powershell
python build.py debug
```

或双击 `build_py.bat`。

## 运行编辑器

1. 在 Visual Studio 中打开 `build/x64-debug` 下生成的解决方案，或将 **HimiiEditor** 设为启动项目。
2. 也可直接运行：

   `build/x64-debug/bin/HimiiEditor/Debug/HimiiEditor.exe`

3. 首次运行前需成功构建 **ScriptCore**（CMake 目标 `ScriptCore_Build`，构建 HimiiEditor 时会自动触发）。

## 输出目录说明

引擎通过 `himii_set_output_dirs` 统一输出到：

- 可执行文件：`${CMAKE_BINARY_DIR}/bin/<TargetName>/<Config>/`
- 静态库 Engine：`${CMAKE_BINARY_DIR}/lib/Engine/<Config>/`

示例（Windows）：

| 配置 | 编辑器 | 运行时 |
|------|--------|--------|
| Debug | `build/x64-debug/bin/HimiiEditor/Debug/HimiiEditor.exe` | `build/x64-debug/bin/HimiiRuntime/Debug/HimiiRuntime.exe` |
| Release | `build/x64-release/bin/HimiiEditor/Release/HimiiEngine.exe` | `build/x64-release/bin/HimiiRuntime/Release/HimiiRuntime.exe` |

Release 下编辑器目标在 CMake 中仍名为 `HimiiEditor`，输出文件名称为 `HimiiEngine.exe`。

Post-build 会将 `assets`、`resources`、`ScriptCore.dll` 等拷贝到 HimiiEditor 输出目录。

## 运行 HimiiRuntime（本地构建）

在完整构建 HimiiRuntime 目标后，可从输出目录启动：

```text
build/x64-debug/bin/HimiiRuntime/Debug/HimiiRuntime.exe --project "D:\path\to\YourProject\YourProject.hproj"
```

- 使用 `--project` 指定 `.hproj` 文件路径。
- 未指定 `--project` 时，在 **HimiiRuntime 可执行文件所在目录** 查找 `.hproj` 文件。

运行前请确认该目录下已有 `ScriptCore.dll`、`ScriptCore.runtimeconfig.json` 以及项目所需的 `assets`（与编辑器 Post-build 拷贝规则相同）。

## 故障排除

* **找不到 ScriptCore.dll**：先完整构建 HimiiEditor，或单独 `dotnet build ScriptCore`。
* **vcpkg 依赖失败**：确认仓库内 `vcpkg` 子模块存在，并重跑 `cmake --preset x64-debug`。
* 更多平台说明见仓库 [README](https://github.com/HimiiFish/Himii-Engine/blob/main/README.md)（含 Linux 与 `build.py` 用法）。
