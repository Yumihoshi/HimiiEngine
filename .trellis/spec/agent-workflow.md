# Agent 工作流与工程约束

> 约束所有在本仓库内编码的 AI Agent（Cursor / Claude Code / Codex 等）。  
> 详细产品 UI 规则见 [product-ux.md](./product-ux.md)。

---

## 语言与命名

- 与开发者沟通、撰写本仓库规范与开发者文档时使用**简体中文**（路径、CLI、目录名保持原文）。
- **严禁缩写命名**：变量、函数、类、常量等须使用完整英文单词（例如用 `index` 而非 `idx`，用 `configuration` 而非 `config` 作为随意缩写）。
- 命名风格遵循各语言惯例（C++ / C# PascalCase 或项目既有风格），但「不缩写」优先。

---

## 改动边界

- **最小必要改动**：不重构无关代码，不顺手「清理」未要求的文件。
- **禁止修改** `Engine/vender/**` 及第三方子模块源码（除非任务明确要求升级依赖，并经开发者确认）。
- **禁止提交**构建与生成产物：`build/`、`bin/`、`obj/`、各项目的 `bin/`/`obj/`、无意义的本地缓存等。
- **禁止擅自** `git commit` / `git push`；仅在开发者明确要求时提交。

---

## 新功能成套交付

新增或显著扩展一个**组件 / 资产类型 / 可序列化字段**时，默认应成套完成（缺一环则说明并征得同意后再省略）：

1. 引擎侧定义（如 `Components.h` / 资产类型）
2. 场景或资产**序列化**读写
3. Editor **Inspector Drawer**（遵守 [product-ux.md](./product-ux.md)）
4. 若对脚本开放：ScriptCore + `ScriptGlue` / `InternalCalls` 成对更新

---

## 编辑时与运行时

- 玩法 / 模拟逻辑放在 Engine / Runtime / 脚本侧，不要塞进 Editor Panel。
- Editor 不得偷偷改写「仅运行时」状态，除非有明确的编辑器预览语义并在 PRD/任务中写明。

---

## 不确定时先问

- 需要新增或扩展**公共 API**（含 `InspectorControls`、Scene、Asset、ScriptGlue 对外面）时：**先提问，通过后再改**。
- 需求、归属层级（Engine / HimiiEditor / ScriptCore / HimiiRuntime）或序列化兼容性不清晰时：先问，不猜测实现。

---

## Trellis 任务

- **非琐碎工作**（新组件、跨多层改动、公共 API、多文件特性）应创建或挂接 `.trellis/tasks/` 下的任务，写入 PRD / 实现上下文，避免无任务上下文散改。
- 简单问答、单点 typo、明确的一行修复可不建任务；拿不准时询问开发者。
- 完成阶段性工作后，优先使用平台上的 Trellis 命令（如 `/trellis-finish-work`、`/trellis:finish-work`）收尾，而不是只丢下一堆未归档改动。

---

## 规范源真相

- 团队共享约束以 `.trellis/spec/` 为准。
- `.cursor/`、`.claude/`、`.codex/`、`.agents/`、根目录 `AGENTS.md` 均为**本机生成物，不入库**；若本地存在 `AGENTS.md`，仅为 Trellis/Codex 入口前言，**不要**在其中堆叠业务规范。
