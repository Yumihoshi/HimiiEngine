namespace HimiiEngine
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class Transform : Component
    {
        public Vector3 Position
        {
            get
            {
                InternalCalls.Transform_GetTranslation(Entity.ID, out Vector3 result);
                return result;
            }
            set => InternalCalls.Transform_SetTranslation(Entity.ID, ref value);
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.Transform_GetRotation(Entity.ID, out Vector3 result);
                return result;
            }
            set => InternalCalls.Transform_SetRotation(Entity.ID, ref value);
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.Transform_GetScale(Entity.ID, out Vector3 result);
                return result;
            }
            set => InternalCalls.Transform_SetScale(Entity.ID, ref value);
        }
    }

    public class SpriteRenderer : Component
    {
        public Vector4 Color
        {
            get
            {
                InternalCalls.SpriteRenderer_GetColor(Entity.ID, out Vector4 result);
                return result;
            }
            set => InternalCalls.SpriteRenderer_SetColor(Entity.ID, ref value);
        }

        /// <summary>子 Sprite 资产句柄（推荐）。</summary>
        public ulong SpriteAssetHandle
        {
            get => InternalCalls.SpriteRenderer_GetSpriteHandle(Entity.ID);
            set => InternalCalls.SpriteRenderer_SetSpriteHandle(Entity.ID, value);
        }

        /// <summary>与 SpriteAssetHandle 相同，保留兼容。</summary>
        public ulong SpriteHandle
        {
            get => SpriteAssetHandle;
            set => SpriteAssetHandle = value;
        }

        /// <summary>设置纹理时自动绑定其默认子 Sprite。</summary>
        public ulong TextureHandle
        {
            get => InternalCalls.SpriteRenderer_GetTextureHandle(Entity.ID);
            set => InternalCalls.SpriteRenderer_SetTextureHandle(Entity.ID, value);
        }
    }

    public class Rigidbody2D : Component
    {
        public Vector2 Velocity
        {
            get
            {
                InternalCalls.Rigidbody2D_GetLinearVelocity(Entity.ID, out Vector2 result);
                return result;
            }
            set => InternalCalls.Rigidbody2D_SetLinearVelocity(Entity.ID, ref value);
        }

        public void ApplyImpulse(Vector2 impulse, Vector2 point, bool wake)
        {
            InternalCalls.Rigidbody2D_ApplyLinearImpulse(Entity.ID, ref impulse, ref point, wake);
        }

        public void ApplyImpulse(Vector2 impulse, bool wake)
        {
            InternalCalls.Rigidbody2D_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
        }
    }
}