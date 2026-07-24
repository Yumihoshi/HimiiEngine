# 产品体验规范（面向引擎用户）

> 本规范约束 Editor / Panel / Inspector 等面向**使用 HimiiEngine 的游戏开发者**的界面与交互。  
> 功能服务于引擎用户，而不是引擎实现者的调试便利。

---

## 核心原则

1. **用户语义优先**：Inspector 与面板只暴露用户可理解的概念（资源名、预览、音量、颜色、枚举含义等）。
2. **隐藏引擎内部实现**：不得把供引擎内部读取的实现细节当作常规可编辑字段画出来。
3. **样式统一**：所有属性绘制必须走 `HimiiEditor` 的 `InspectorControls` API。

---

## 禁止暴露的内部值（示例）

| 禁止在常规 Inspector 中直接展示/编辑 | 正确做法 |
|---|---|
| 原始 `AssetHandle` 数值（如 `uint64` ID） | 显示资源文件名 / Sprite 名；用拖放与引用字段赋值 |
| 内部指针、原生对象地址、调试用原始 ID | 仅日志或专用调试面板（若团队明确需要） |
| 仅运行时有效、用户无法理解的缓存字段 | 不绘制；或由引擎侧自动维护 |

**正例**：`DrawObjectReferenceField` + `AssignSpriteFromContentBrowserPayload` 等，展示名称与预览，拖放赋值 Handle。  
**反例**：在 Drawer 里 `ImGui::Text("%llu", handle)` 或把 Handle 当数字输入框。

---

## InspectorControls 强制约定

- Panel / Component Inspector Drawer **必须**使用 `HimiiEditor/src/InspectorControls.h` 中已有控件（如 `DrawFloatControl`、`DrawObjectReferenceField`、`DrawEnumComboControl` 等）。
- **禁止**在 Drawer 中随意拼裸 `ImGui` 控件来绕过统一样式（拖放目标等已封装在 Controls 内的能力除外；若必须扩展，见下条）。
- 若现有 Controls **无法**满足交互：
  1. **先向开发者提问**，说明缺什么样式/交互；
  2. 获得明确同意后，再向 `InspectorControls` **新增**可复用 API；
  3. 禁止只在单个 Drawer 里私自发明一套 UI。

---

## 文案与可发现性（短约束）

- UI 标签使用**产品语义**（如 “Sprite”“Volume”），不要把引擎类型名当作唯一用户文案。
- 清空引用、删除组件等破坏性操作应有明确控件与可预期结果，避免静默改写用户资产。

---

## 适用范围

- `HimiiEditor/src/panel/**`
- `HimiiEditor/src/InspectorControls.*`
- 任何会写入场景/资产、并呈现给引擎用户的编辑器 UI
