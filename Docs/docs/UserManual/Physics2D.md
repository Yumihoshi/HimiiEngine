# 2D 物理系统 (Physics 2D)

HimiiEngine 内置 **Box2D v3** 作为 2D 物理引擎。

## 启用物理

为实体添加以下组件（通常同时需要 **Transform**）：

### Rigidbody 2D

定义刚体类型与旋转锁定。

| Body Type | 说明 |
|-----------|------|
| **Static** | 不移动（地面、墙等） |
| **Dynamic** | 受力与碰撞影响（玩家、可推动物体） |
| **Kinematic** | 主要由脚本/编辑器驱动，模拟行为与 Dynamic 不同 |

- **Fixed Rotation**：勾选后锁定 Z 轴旋转。

### Box Collider 2D

矩形碰撞形状。

- **Offset** / **Size**：相对 Transform 的偏移与尺寸。
- **Density**、**Friction**、**Restitution**、**Restitution Threshold**：密度、摩擦、弹性及弹性阈值。

### Circle Collider 2D

圆形碰撞形状。

- **Offset**、**Radius**：圆心与半径。
- 材料参数同 Box Collider。

至少一方需有 **Rigidbody2D**，碰撞体才会参与模拟。进入 **Play** 或 **Simulate** 时创建物理世界；**Stop** 后销毁。

## 脚本控制刚体

实体上若有 `Rigidbody2D` 组件，可通过脚本访问：

```csharp
var rb = GetComponent<Rigidbody2D>();
if (rb != null)
{
    rb.Velocity = new Vector2(3.0f, 0.0f);
    rb.ApplyImpulse(new Vector2(0.0f, 5.0f), wake: true);
}
```

Transform 与刚体位置在物理步进后会同步（Dynamic 物体以物理结果为准）。

## 碰撞事件（脚本）

在 **Play** 模式下，两个形状开始/结束接触时，引擎会调用脚本虚方法（双方实体若挂有 `ScriptComponent` 且已实例化，各收到一次回调，对方 ID 在 `Collision2DInfo.OtherEntityID` 中）：

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

注意：

- 仅在 **Play** 模式触发（**Simulate** 不运行脚本实例）。
- 双方需有碰撞体；动态交互至少一方通常为 **Dynamic** 或 **Kinematic**。
- 使用 `Log` 可在 **Console** 面板查看输出。

## 与编辑器的配合

- **Edit**：编辑碰撞体参数，不运行物理。
- **Simulate**：预览物理，不调用 `OnCreate` / `OnUpdate` / 碰撞脚本。
- **Play**：完整运行时（脚本 + 物理 + 碰撞回调）。
