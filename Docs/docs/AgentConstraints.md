# Agent 约束与 Trellis 协作

面向引擎贡献者：如何在本仓库使用 **mindfold-ai/Trellis**，让 Cursor、Claude Code、Codex 及其他 AI IDE / 终端共享同一套约束与任务记忆。

---

## 设计摘要

| 项 | 说明 |
|----|------|
| 规范源真相 | `.trellis/spec/`（当前主要为 `product-ux.md`、`agent-workflow.md`） |
| 工作流 | `.trellis/workflow.md` + `.trellis/tasks/` |
| 个人记忆 | `.trellis/workspace/<你的名字>/` |
| Git 策略 | **只提交** `.trellis/` 与文档；各 IDE 适配目录在本机生成，**不入库** |
| 推荐本机平台 | 按需 init；仓库维护者常用 **Cursor** 即可 |

业务规则只维护在 `.trellis/spec/`。`.cursor/`、`.claude/`、`.codex/`、`.agents/`、根目录 `AGENTS.md` 均视为生成物，可随本机 `trellis init` / `trellis update` 更新，**不要提交**。

根目录出现多个 AI 相关文件夹是各工具的固定约定路径，**不能**合并到子目录，否则 IDE 无法发现配置。本仓库用「不入库」来控制 Git/PR 噪声，而不是搬家。

---

## 环境要求

- Node.js ≥ 18
- Python ≥ 3.9
- 建议用 `npx` 调用 CLI，避免强制全局安装：

```bash
npx --yes @mindfoldhq/trellis@latest --version
```

也可自行全局安装：`npm install -g @mindfoldhq/trellis@latest`。

---

## 新成员加入本仓库

1. 拉取含 `.trellis/` 的仓库。
2. 为本机建立身份，并**只生成你实际使用的平台**：

```bash
cd Himii-Engine
# 仅 Cursor
npx --yes @mindfoldhq/trellis@latest init -u 你的名字 --cursor --no-monorepo -y

# 若还用 Claude Code / Codex，追加对应标志即可，例如：
# npx --yes @mindfoldhq/trellis@latest init --claude -y
# npx --yes @mindfoldhq/trellis@latest init --codex -y
```

已有 `.trellis/` 时，CLI 会走「加入已有项目 / 追加平台」，**不要**选 Full re-initialize。

3. 在所用 AI 工具中开新会话；Cursor 可用 `/trellis-finish-work`、`/trellis-continue` 等命令（以本机 `.cursor/commands/` 为准）。

---

## 必读规范

动手改 Editor / 组件 / 脚本桥之前，至少阅读：

1. [`.trellis/spec/product-ux.md`](../../.trellis/spec/product-ux.md) — 面向用户的 Inspector；必须用 `InspectorControls`
2. [`.trellis/spec/agent-workflow.md`](../../.trellis/spec/agent-workflow.md) — 命名、成套交付、先问后改、Trellis 任务

---

## Git 约定（本仓库）

| 路径 | 是否提交 |
|------|----------|
| `.trellis/spec/`、`workflow.md`、`config.yaml`、`scripts/`、`tasks/` | **提交** |
| `.trellis/.developer`、`.trellis/.runtime/` | **不提交**（`.trellis/.gitignore`） |
| `.trellis/workspace/<他人>/` | **不要擅自提交他人日志** |
| `.cursor/`、`.claude/`、`.codex/`、`.agents/`、`AGENTS.md` | **不提交**（根 `.gitignore`） |

本仓库已将 Trellis `session_auto_commit` 设为 `false`，避免 Agent 自动产生 git commit。

---

## 各平台本机怎么用

### Cursor（推荐默认）

```bash
npx --yes @mindfoldhq/trellis@latest init -u 你的名字 --cursor --no-monorepo -y
```

- 命令与 hooks 在本机 `.cursor/`（已 gitignore）
- 规范仍从 `.trellis/spec/` 注入；无需把 `.cursor` 推上远程

### Claude Code

```bash
npx --yes @mindfoldhq/trellis@latest init --claude -y
```

配置在本机 `.claude/`。新会话应由 SessionStart hook 注入上下文。

### Codex

```bash
npx --yes @mindfoldhq/trellis@latest init --codex -y
```

会生成本机 `AGENTS.md`、`.codex/`、`.agents/skills/`（均不入库）。建议在 `~/.codex/config.toml` 启用：

```toml
[features]
hooks = true
```

Codex 0.129+ 还需在会话中执行一次 `/hooks`，批准 Trellis 的 hook。

---

## 如何为其他 AI IDE 添加约束支持

目标：让新工具也读到**同一套** `.trellis/spec/`，而不是复制规则，也**不要**把新平台目录提交进库。

### 步骤

1. 确认 Trellis 是否已有该平台适配（见 [Multi-Platform](https://docs.trytrellis.app/advanced/multi-platform)）。
2. 在本机追加平台（不要 Full re-initialize）：

```bash
npx --yes @mindfoldhq/trellis@latest init --<平台标志> -y
```

3. 本地验证：新开会话能读到 `.trellis/workflow.md` 与 `.trellis/spec/`。
4. **不要**把新生成的 `.<平台>/` 或 `AGENTS.md` 加入 Git；若文档需补充「如何 init 该平台」，只改 `Docs/docs/AgentConstraints.md`。
5. 没有官方适配时：仍以 `.trellis/spec/` 为唯一业务规范源；工具自带说明文件里只写短入口指向 `.trellis/spec/index.md`。

### 禁止事项

- 不要把 `product-ux` / `agent-workflow` 全文复制到多个入口文件导致多源漂移
- 不要试图把 `.cursor` / `.claude` 等挪到子目录「整理观感」——会破坏 IDE 发现路径
- 不要为了「方便」在 Drawer 里绕过 `InspectorControls`
- 不要修改 `Engine/vender/**` 来迁就 Agent

---

## 升级 Trellis

```bash
npx --yes @mindfoldhq/trellis@latest upgrade   # 如使用全局 CLI
npx --yes @mindfoldhq/trellis@latest update    # 同步本机模板与已 init 的平台文件
```

`update` 可能改写本机平台目录；升级后检查 `.trellis/spec/` 中的 Himii 自定义文件未被覆盖。平台目录变更仍留在本机，勿提交。

---

## 相关链接

- 官方文档：https://docs.trytrellis.app/
- 仓库 Spec 索引：[`.trellis/spec/index.md`](../../.trellis/spec/index.md)
- 开发者手册目录：[DevManual/README.md](DevManual/README.md)
