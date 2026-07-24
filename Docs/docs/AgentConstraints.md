# Agent 约束与 Trellis 协作

面向引擎贡献者：如何在本仓库使用 **mindfold-ai/Trellis**，让 Cursor、Claude Code、Codex 及其他 AI IDE / 终端共享同一套约束与任务记忆。

---

## 设计摘要

| 项 | 说明 |
|----|------|
| 规范源真相 | `.trellis/spec/`（当前主要为 `product-ux.md`、`agent-workflow.md`） |
| 工作流 | `.trellis/workflow.md` + `.trellis/tasks/` |
| 个人记忆 | `.trellis/workspace/<你的名字>/` |
| 已预配置平台 | **Cursor**、**Claude Code**、**Codex** |
| 根目录 `AGENTS.md` | 由 Trellis 生成的入口前言（尤其服务 Codex）；**不要**在其中堆业务规则 |

业务规则只维护在 `.trellis/spec/`。平台目录（`.cursor/` 中 Trellis 部分、`.claude/`、`.codex/`、`.agents/skills/`）为生成物，可随 `trellis update` 更新。

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

仓库已完成 `trellis init`。你在本机首次使用时：

```bash
cd Himii-Engine
npx --yes @mindfoldhq/trellis@latest init -u 你的名字 -y
```

CLI 会检测到已有 `.trellis/`，并为你生成本机身份（`.trellis/.developer`，已 gitignore）与个人 workspace 目录。

然后在所用 AI 工具中开启新会话；若平台提供 Trellis 命令，可用 `/trellis-start` 或 `/trellis:start`（名称因平台而异）确认上下文已注入。

---

## 必读规范

动手改 Editor / 组件 / 脚本桥之前，至少阅读：

1. [`.trellis/spec/product-ux.md`](../../.trellis/spec/product-ux.md) — 面向用户的 Inspector；必须用 `InspectorControls`
2. [`.trellis/spec/agent-workflow.md`](../../.trellis/spec/agent-workflow.md) — 命名、成套交付、先问后改、Trellis 任务

---

## Git 约定（本仓库）

| 路径 | 是否提交 |
|------|----------|
| `.trellis/spec/`、`workflow.md`、`config.yaml`、`tasks/` | **提交** |
| `.trellis/scripts/`、平台生成物（见下） | **提交** |
| `.trellis/.developer`、`.trellis/.runtime/` | **不提交**（已由 `.trellis/.gitignore` 忽略） |
| `.trellis/workspace/<他人>/` | **不要擅自提交他人日志**；只维护自己的目录 |
| `HimiiEditor` 相关：`.cursor/agents|commands|hooks|skills` | **提交**（根 `.gitignore` 已放行这些 Trellis 路径） |
| `.claude/`、`.codex/`、`.agents/skills/`、`AGENTS.md` | **提交** |

本仓库已将 Trellis `session_auto_commit` 设为 `false`，避免 Agent 自动产生 git commit。

---

## 已支持平台怎么用

### Cursor

- 命令：`.cursor/commands/`（如 `/trellis-start`、`/trellis-finish-work`）
- Skills / Agents / Hooks：由 Trellis 写入 `.cursor/`
- 确保使用的是仓库内提交的 Trellis 集成文件，而不是本机另起一套无关 rules

### Claude Code

- 配置在 `.claude/`（commands、agents、skills、hooks、`settings.json`）
- 新会话应由 SessionStart hook 注入 Trellis 上下文

### Codex

- 入口：`AGENTS.md`（自动读取）
- 配置：`.codex/` 与跨平台 `.agents/skills/`
- **本机额外一步**（否则 hooks/命令不完整）：在 `~/.codex/config.toml` 中启用：

```toml
[features]
hooks = true
```

Codex 0.129+ 还需在会话中执行一次 `/hooks`，批准 Trellis 的 hook。

---

## 如何为其他 AI IDE 添加约束支持

目标：让新工具也读到**同一套** `.trellis/spec/`，而不是复制一份规则到别处。

### 步骤

1. **确认 Trellis 是否已有该平台适配**  
   官方支持标志见 [Multi-Platform 文档](https://docs.trytrellis.app/advanced/multi-platform)。常见标志包括：`--gemini`、`--opencode`、`--copilot`、`--kiro`、`--qoder` 等。

2. **在本仓库追加平台（不要 Full re-initialize）**

```bash
npx --yes @mindfoldhq/trellis@latest init --<平台标志> -y
```

例如只加 Gemini CLI：

```bash
npx --yes @mindfoldhq/trellis@latest init --gemini -y
```

选择交互菜单里的 **Add AI platform(s)** 也可以；**不要**选 Full re-initialize，以免覆盖团队已有配置。

3. **本地验证**  
   - 新开该 IDE / CLI 的 Agent 会话  
   - 确认能读到 `.trellis/workflow.md` 与 `.trellis/spec/`  
   - 若有 `/trellis-*` 或等价命令，跑一遍 start / 简单问答

4. **决定是否提交生成物**  
   - 若该平台会在团队中长期使用：将新生成的平台目录一并提交，并在本页「已支持平台」中补一小节  
   - 若仅个人试用：可先不提交，避免无关 diff；试用通过后再开 PR

5. **没有官方适配时**  
   - 仍以 `.trellis/spec/` 为唯一业务规范源  
   - 在该工具自己的项目说明文件里**只写短入口**（指向 `.trellis/spec/index.md` 与 `workflow.md`），不要复制全文  
   - 若工具支持 [agentskills.io](https://agentskills.io) 的 `.agents/skills/`，可直接复用仓库中已提交的 skills

### 禁止事项

- 不要把 `product-ux` / `agent-workflow` 全文复制到多个 `CLAUDE.md` / `.cursorrules` 导致多源漂移  
- 不要为了「方便」在 Drawer 里绕过 `InspectorControls`  
- 不要修改 `Engine/vender/**` 来迁就 Agent

---

## 升级 Trellis

```bash
npx --yes @mindfoldhq/trellis@latest upgrade   # 如使用全局 CLI，则升级全局包
npx --yes @mindfoldhq/trellis@latest update    # 同步本仓库模板与平台文件
```

若提示需要 migration：`trellis update --migrate`。升级后检查 `.trellis/spec/` 中的 Himii 自定义文件未被覆盖；若冲突，保留 `product-ux.md` / `agent-workflow.md` 的内容。

---

## 相关链接

- 官方文档：https://docs.trytrellis.app/
- 仓库 Spec 索引：[`.trellis/spec/index.md`](../../.trellis/spec/index.md)
- 开发者手册目录：[DevManual/README.md](DevManual/README.md)
