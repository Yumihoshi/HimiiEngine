using System;

namespace HimiiEngine
{
    /// <summary>
    /// 标记 private 实例字段参与 Inspector 反射与场景序列化（与 public 实例字段行为一致）。
    /// </summary>
    /// <example>
    /// <code>
    /// [SerializeField]
    /// private float moveSpeed = 5.0f;
    /// </code>
    /// </example>
    [AttributeUsage(AttributeTargets.Field)]
    public sealed class SerializeField : Attribute
    {
    }
}
