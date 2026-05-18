# 脚本 API (Scripting API)

HimiiEngine 使用 **C#** 作为游戏脚本语言，运行时基于 **.NET 8 + CoreCLR**（非 Mono）。脚本编译为 `GameAssembly.dll`，由引擎通过可收集的 `AssemblyLoadContext` 加载。

## 创建脚本

1. 在 **Content Browser** 中右键 → Create → C# Script。
2. 命名脚本（例如 `PlayerController.cs`）。
3. 在属性面板为实体添加 **Script Component**，填写完整类名（含命名空间）。
4. 点击 **Edit** 或菜单 **File → Open C# Project** 在外部 IDE 中编辑（Visual Studio / VS Code / Rider，见 [脚本工作流](ScriptWorkflow.md)）。

## Entity 生命周期

继承自 `Himii.Entity` 的类可重写以下虚方法（须使用 `public override`）：

```csharp
using Himii;

public class PlayerController : Entity
{
    public override void OnCreate()
    {
        Log.Info($"Entity {ID} created.");
    }

    public override void OnUpdate(float ts)
    {
        // ts：上一帧时间间隔（秒），用于帧率无关逻辑
        Position += new Vector3(1.0f, 0.0f, 0.0f) * ts;
    }

    public override void OnDestroy()
    {
        Log.Info("Entity destroyed.");
    }
}
```

生命周期仅在 **Play** 模式下、且实体带有有效 `ScriptComponent` 时由引擎调用。

## 日志 (Log)

使用 `Himii.Log` 将消息写入引擎日志与编辑器 **Console** 面板（**Window → Console**）：

```csharp
Log.Info("Game started.");
Log.Warning("Low health!");
Log.Error("Something went wrong.");
```

- 输出同时出现在控制台（spdlog）与编辑器 **Console**。
- 进入 **Play** 时会清空 Console 缓冲，避免与上次运行混淆。
- **Script Console**（**Window → Script Console**）仅显示 `dotnet build` 编译输出，与运行时 `Log` 不同。

请勿依赖 `Console.WriteLine` 作为游戏日志手段（不会进入编辑器 Console）。

## 输入 (Input)

```csharp
if (Input.IsKeyDown(KeyCode.W))
{
    // 按键按住
}

Vector2 mouse = Input.MousePosition;
if (Input.IsMouseButtonDown(0))
{
    // 左键
}
```

## Transform 与组件

每个实体均有 `Transform`（通过 `entity.Transform` 或 `Position` / `Rotation` / `Scale` 访问）：

```csharp
Position += new Vector3(1.0f, 0.0f, 0.0f) * ts;
Transform.Scale = new Vector3(2.0f, 2.0f, 1.0f);
```

获取其他组件（需实体上已添加对应 C++ 组件）：

```csharp
var rb = GetComponent<Rigidbody2D>();
if (rb != null)
    rb.Velocity = new Vector2(5.0f, 0.0f);
```

## 2D 物理碰撞

在 **Play** 模式下，当带碰撞体的实体发生接触/分离时，可重写：

```csharp
public override void OnCollisionEnter2D(Collision2DInfo collision)
{
    Log.Info($"Hit entity {collision.OtherEntityID}");
}

public override void OnCollisionExit2D(Collision2DInfo collision)
{
    Log.Info($"Left entity {collision.OtherEntityID}");
}
```

要求：双方实体均有 **Rigidbody2D** 与 **BoxCollider2D** 或 **CircleCollider2D**；仅在运行时 **Play** 模式触发（非纯 Simulate 脚本实例）。

更多物理组件说明见 [2D 物理](Physics2D.md)。

## Inspector 脚本字段

在脚本类中声明 **public 实例字段**，可在属性面板中编辑并随场景 YAML 保存：

```csharp
public float Speed = 5.0f;
public Entity Target;  // 引用场景中另一实体（UUID）
```

- 支持类型：数值、布尔、`Vector2/3/4`、`string`、`KeyCode`、`Entity` 等（与引擎 `ReflectionBridge` 一致）。
- 修改后保存场景即可持久化。
- `[SerializeField]` 与 private 字段尚未支持。

## 相关文档

- [脚本工作流](ScriptWorkflow.md)：编译、文件监视、IDE 配置
- [编辑器界面](EditorInterface.md)：Console、Script Console、Play 工具栏
- [2D 物理](Physics2D.md)：刚体、碰撞体与碰撞事件
