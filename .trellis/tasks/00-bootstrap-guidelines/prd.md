# Bootstrap Task: Fill Project Development Guidelines

**状态：已由团队完成首版 Himii 规范，无需再按 frontend/backend 脚手架填写。**

本仓库已用以下文件替换默认前后端占位规范：

| 文件 | 内容 |
|------|------|
| `.trellis/spec/product-ux.md` | 面向引擎用户的 Editor / Inspector；`InspectorControls` |
| `.trellis/spec/agent-workflow.md` | Agent 工作流、命名、成套交付、Trellis 任务 |
| `.trellis/spec/index.md` | Spec 索引 |
| `Docs/docs/AgentConstraints.md` | 开发者文档（多平台与扩展说明） |

## Status

- [x] 写入 Himii 产品体验规范
- [x] 写入 Agent 工作流规范
- [x] 移除不适用的 frontend/backend 占位指南
- [x] 文档说明 Cursor / Claude Code / Codex 与扩展其他 IDE 的方式

后续若要扩展规范，直接改 `.trellis/spec/` 并开 PR，不要 Full re-initialize。
