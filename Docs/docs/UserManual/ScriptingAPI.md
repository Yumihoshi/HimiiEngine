# 脚本 API (Scripting API)

HimiiEngine 使用 **C#**（**.NET 8 + CoreCLR**）编写游戏逻辑。脚本编译为项目中的 **`GameAssembly.dll`**，由编辑器在 **Play** 时加载。所有公开 API 位于命名空间 **`HimiiEngine`**（引擎提供 **`ScriptCore.dll`**）。

```csharp
using HimiiEngine;
```

编写与编译流程见 [脚本工作流](ScriptWorkflow.md)；首个完整示例见 [教程：2D 平台跳跃](Tutorial2DPlatformer.md)。

---

## API 索引

| 类别 | 类型 / 成员 | 说明 |
|------|-------------|------|
| 基类 | `Entity` | 脚本实体基类 |
| 生命周期 | `OnCreate` / `OnUpdate` / `OnDestroy` | 仅 Play 模式调用 |
| 日志 | `Log.Info` / `Warning` / `Error` | 输出到 Console |
| 输入 | `Input.IsKeyDown` / `IsMouseButtonDown` / `MousePosition` | 键盘与鼠标 |
| 变换 | `Position` / `Transform` | 实体位置、旋转、缩放 |
| 组件 | `GetComponent<T>()` / `HasComponent<T>()` | 访问引擎组件 |
| 2D 物理 | `Rigidbody2D` | 速度、冲量 |
| 渲染 | `SpriteRenderer` | 颜色、纹理、水平翻转 |
| 动画 | `SpriteAnimation` | `Play` / `CurrentAnimation` |
| 碰撞 | `OnCollisionEnter2D` / `OnCollisionExit2D` | Play 模式回调 |
| 序列化 | `[SerializeField]` | Inspector 显示 private 字段 |

---

## 创建与挂载脚本

1. **Content Browser** → `assets/scripts` → 右键 **Create → C# Script**。
2. 类继承 **`Entity`**，例如 `public class PlayerController : Entity`。
3. 在场景中选中实体 → **Add Component → Script** → **Class Name** 填写类名（与 `.cs` 中一致，区分大小写）。
4. **File → Open C# Project** 在外部 IDE 编辑；保存后 **Script Console** 显示编译结果。

---

## Entity 生命周期

仅在 **Play** 模式、且实体带有有效 **Script** 组件时调用：

```csharp
using HimiiEngine;

public class PlayerController : Entity
{
    public override void OnCreate()
    {
        Log.Info($"Entity {ID} created.");
    }

    public override void OnUpdate(float timestep)
    {
        // timestep：上一帧间隔（秒），用于帧率无关移动
        Position += new Vector3(1.0f, 0.0f, 0.0f) * timestep;
    }

    public override void OnDestroy()
    {
        Log.Info("Entity destroyed.");
    }
}
```

| 方法 | 调用时机 |
|------|----------|
| `OnCreate` | 实体脚本实例化后一次 |
| `OnUpdate(timestep)` | 每帧 |
| `OnDestroy` | 实体销毁或 Stop 时 |

---

## 日志 (Log)

```csharp
Log.Info("Game started.");
Log.Warning("Low health!");
Log.Error("Something went wrong.");
```

- 显示在 **Window → Console**（进入 Play 时会清空缓冲）。
- **Script Console** 仅显示编译输出，与 `Log` 无关。
- 请勿依赖 `Console.WriteLine` 作为游戏日志（不会进入编辑器 Console）。

---

## 输入 (Input)

```csharp
if (Input.IsKeyDown(KeyCode.W))
{
    // 键按住
}

if (Input.IsKeyDown(KeyCode.Space))
{
    // 跳跃等
}

Vector2 mouse = Input.MousePosition;
if (Input.IsMouseButtonDown(0))
{
    // 左键
}
```

常用键码定义在 **`KeyCode`** 枚举（`A`、`D`、`Left`、`Right`、`Space` 等），完整列表见 `ScriptCore` 中 `KeyCode.cs`。

---

## Transform 与位置

每个实体均有 **Transform**。可通过实体快捷属性或组件访问：

```csharp
Position += new Vector3(1.0f, 0.0f, 0.0f) * timestep;

Transform.Scale = new Vector3(2.0f, 2.0f, 1.0f);
Transform.Rotation = new Vector3(0.0f, 0.0f, 0.0f);
```

**注意**：角色左右朝向请用 **`SpriteRenderer.FlipHorizontal`**，不要将 **`Transform.Scale.x`** 设为负数（会导致碰撞与渲染异常）。

---

## 组件访问

```csharp
var rigidbody = GetComponent<Rigidbody2D>();
if (rigidbody != null)
    rigidbody.Velocity = new Vector2(5.0f, 0.0f);

bool hasAnimation = HasComponent<SpriteAnimation>();
```

`GetComponent<T>()` 在实体未挂载该组件时返回 `null`。

### 静态工厂（可选）

```csharp
Entity player = Entity.Find("Player");
Entity spawned = Entity.Create("Enemy");
Entity.Destroy(spawned);
```

---

## Rigidbody2D

```csharp
var rigidbody = GetComponent<Rigidbody2D>();
if (rigidbody != null)
{
    rigidbody.Velocity = new Vector2(3.0f, 0.0f);
    rigidbody.ApplyImpulse(new Vector2(0.0f, 5.0f), wake: true);
}
```

| 成员 | 说明 |
|------|------|
| `Velocity` | 线速度（`Vector2`） |
| `ApplyImpulse(impulse, wake)` | 对质心施加冲量 |
| `ApplyImpulse(impulse, point, wake)` | 在世界坐标点施加冲量 |

Inspector 中设置 **Body Type**（Static / Dynamic / Kinematic）、**Fixed Rotation** 等。详见 [2D 物理](Physics2D.md)。

---

## SpriteRenderer

```csharp
var spriteRenderer = GetComponent<SpriteRenderer>();
spriteRenderer.FlipHorizontal = true;
spriteRenderer.Color = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
spriteRenderer.TextureHandle = textureHandle;
```

| 属性 | 说明 |
|------|------|
| `Color` | `Vector4` 着色 |
| `SpriteAssetHandle` / `SpriteHandle` | 子 Sprite 资产句柄 |
| `TextureHandle` | 纹理句柄（可自动绑定默认子 Sprite） |
| `FlipHorizontal` | 水平镜像绘制 |

---

## SpriteAnimation

```csharp
var animation = GetComponent<SpriteAnimation>();
animation.Play("Run");
animation.CurrentAnimation = "Idle";
animation.FrameRate = 12.0f;
animation.Playing = true;
animation.Stop();
```

| API | 说明 |
|-----|------|
| `Play()` | 播放当前 `CurrentAnimation`，重置帧 |
| `Play(string animationName)` | 切换命名动画并播放 |
| `CurrentAnimation` | 与 `.anim` 内动画 **Name** 一致（区分大小写） |
| `Playing` | 是否推进帧 |
| `FrameRate` | 大于 0 时覆盖资产内默认 FPS |
| `Stop()` | 停止播放 |

资产与编辑器说明见 [2D 逐帧动画](SpriteAnimation.md)。

---

## 2D 碰撞回调

在 **Play** 模式下，碰撞开始/结束时调用（双方若挂 Script 且已实例化，各收到一次回调）：

```csharp
public override void OnCollisionEnter2D(Collision2DInfo collision)
{
    Log.Info($"Enter: {collision.OtherEntityID}");
}

public override void OnCollisionExit2D(Collision2DInfo collision)
{
    Log.Info($"Exit: {collision.OtherEntityID}");
}
```

| 说明 | 细节 |
|------|------|
| 触发条件 | 双方有 **Box** 或 **Circle Collider 2D**；至少一方参与动态模拟 |
| 不触发 | **Simulate** 模式不运行脚本实例 |
| 用途 | 着地检测、伤害判定等 |

`Collision2DInfo.OtherEntityID` 为对方实体 ID。更多组件参数见 [2D 物理](Physics2D.md)。

---

## Inspector 与 [SerializeField]

**public 实例字段** 或带 **`[SerializeField]`** 的 **private 实例字段** 会出现在属性面板，并随场景 YAML 的 `ScriptFields` 保存：

```csharp
using HimiiEngine;

public class PlayerController : Entity
{
    public float MoveSpeed = 5.0f;
    public Entity Target;

    [SerializeField]
    private float jumpSpeed = 7.0f;
}
```

`[SerializeField]` 类型定义在 `assets/scripts/Himii/SerializeField.cs`（引擎同步到游戏项目）。

| 规则 | 说明 |
|------|------|
| 命名空间 | 特性在 **`HimiiEngine`**，脚本需 `using HimiiEngine;` |
| 支持类型 | 数值、布尔、`Vector2/3/4`、`string`、`KeyCode`、`Entity` 等 |
| 不支持 | static 字段、属性（Property） |
| 持久化 | 修改后 **Ctrl+S** 保存场景 |
| 不显示 | 检查类名、重新编译 ScriptCore、重启编辑器后重选实体 |

> **【配图占位】** `images/inspector-script-serializefield.png`

---

## 相关文档

- [教程：2D 平台跳跃](Tutorial2DPlatformer.md)
- [编辑器与功能](EditorFeatures.md)
- [脚本工作流](ScriptWorkflow.md)
- [编辑器界面](EditorInterface.md)
- [2D 逐帧动画](SpriteAnimation.md)
- [2D 物理](Physics2D.md)
