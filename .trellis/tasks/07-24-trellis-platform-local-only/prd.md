# 平台适配本地化：Git 只保留 .trellis

## 背景

多平台 Trellis 适配在仓库根产生 `.cursor` / `.claude` / `.codex` / `.agents`，无法物理合并。为降低 Git/PR 噪声与维护成本，改为「规范进库、适配本机生成」。

## 接受标准

- [x] 根 `.gitignore` 忽略 `.cursor/`、`.claude/`、`.codex/`、`.agents/`、`AGENTS.md`
- [x] `Docs/docs/AgentConstraints.md` 反映新策略（不提交平台生成物；文档说明如何本地 init）
- [x] 本机删除非 Cursor 平台目录与 `AGENTS.md`，保留 `.cursor/`
- [x] 仍提交 `.trellis/`（含 Himii spec）作为共享约束源

## 非目标

- 不把平台目录搬到子路径
- 不移除 Cursor 本机 Trellis 集成
