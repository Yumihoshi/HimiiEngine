using System;
using System.Runtime.InteropServices;

namespace HimiiEngine
{
	[StructLayout(LayoutKind.Sequential)]
	internal struct NativeFunctionsMap
	{
		public IntPtr LogFunc;

		// Scene/Entity
		public IntPtr Entity_HasComponent;
		public IntPtr Scene_CreateEntity;
        public IntPtr Scene_DestroyEntity;
        public IntPtr Scene_FindEntityByName;

        // Transform
        public IntPtr Transform_GetTranslation;
		public IntPtr Transform_SetTranslation;
		public IntPtr Transform_GetRotation;
        public IntPtr Transform_SetRotation;
        public IntPtr Transform_GetScale;
        public IntPtr Transform_SetScale;

        // Input
        public IntPtr Input_IsKeyDown;
        public IntPtr Input_IsMouseButtonDown;
        public IntPtr Input_GetMousePosition;

        // Rigidbody2D
        public IntPtr Rigidbody2D_ApplyLinearImpulse;
		public IntPtr Rigidbody2D_ApplyLinearImpulseToCenter;
        public IntPtr Rigidbody2D_GetLinearVelocity;
        public IntPtr Rigidbody2D_SetLinearVelocity;

        public IntPtr BoxCollider2D_GetOffset;
        public IntPtr BoxCollider2D_SetOffset;
        public IntPtr BoxCollider2D_GetSize;
        public IntPtr BoxCollider2D_SetSize;
        public IntPtr BoxCollider2D_GetDensity;
        public IntPtr BoxCollider2D_SetDensity;
        public IntPtr BoxCollider2D_GetFriction;
        public IntPtr BoxCollider2D_SetFriction;
        public IntPtr BoxCollider2D_GetRestitution;
        public IntPtr BoxCollider2D_SetRestitution;

        public IntPtr CircleCollider2D_GetOffset;
        public IntPtr CircleCollider2D_SetOffset;
        public IntPtr CircleCollider2D_GetRadius;
        public IntPtr CircleCollider2D_SetRadius;
        public IntPtr CircleCollider2D_GetDensity;
        public IntPtr CircleCollider2D_SetDensity;
        public IntPtr CircleCollider2D_GetFriction;
        public IntPtr CircleCollider2D_SetFriction;
        public IntPtr CircleCollider2D_GetRestitution;
        public IntPtr CircleCollider2D_SetRestitution;

        // Tilemap
        public IntPtr Tilemap_GetSize;
        public IntPtr Tilemap_SetSize;
        public IntPtr Tilemap_GetTile;
        public IntPtr Tilemap_SetTile;

        // Physics2D
        public IntPtr Physics2D_Raycast;

        // Time
        public IntPtr Time_GetDeltaTime;

        // Scene
        public IntPtr SceneManager_LoadScene;

        // SpriteRenderer
        public IntPtr SpriteRenderer_GetColor;
        public IntPtr SpriteRenderer_SetColor;
        public IntPtr SpriteRenderer_GetSpriteHandle;
        public IntPtr SpriteRenderer_SetSpriteHandle;
        public IntPtr SpriteRenderer_GetTextureHandle;
        public IntPtr SpriteRenderer_SetTextureHandle;
        public IntPtr SpriteRenderer_GetFlipHorizontal;
        public IntPtr SpriteRenderer_SetFlipHorizontal;

        public IntPtr SpriteAnimation_GetPlaying;
        public IntPtr SpriteAnimation_SetPlaying;
        public IntPtr SpriteAnimation_GetFrameRate;
        public IntPtr SpriteAnimation_SetFrameRate;
        public IntPtr SpriteAnimation_ResetPlayback;
        public IntPtr SpriteAnimation_Play;
        public IntPtr SpriteAnimation_GetCurrentAnimationName;
        public IntPtr SpriteAnimation_SetCurrentAnimationName;

        public IntPtr Tilemap_GetBounds;

        public IntPtr Input_IsKeyPressed;
        public IntPtr Input_IsKeyReleased;
        public IntPtr Input_GetAxisHorizontal;
        public IntPtr Input_GetAxisVertical;

        public IntPtr Rigidbody2D_GetBodyType;
        public IntPtr Rigidbody2D_SetBodyType;
        public IntPtr Rigidbody2D_GetFixedRotation;
        public IntPtr Rigidbody2D_SetFixedRotation;

        public IntPtr BoxCollider2D_GetIsTrigger;
        public IntPtr BoxCollider2D_SetIsTrigger;
        public IntPtr BoxCollider2D_GetLayer;
        public IntPtr BoxCollider2D_SetLayer;

        public IntPtr CircleCollider2D_GetIsTrigger;
        public IntPtr CircleCollider2D_SetIsTrigger;
        public IntPtr CircleCollider2D_GetLayer;
        public IntPtr CircleCollider2D_SetLayer;

        public IntPtr SpriteRenderer_GetSortingLayer;
        public IntPtr SpriteRenderer_SetSortingLayer;
        public IntPtr SpriteRenderer_GetSortingOrder;
        public IntPtr SpriteRenderer_SetSortingOrder;

        public IntPtr Camera_GetOrthographicSize;
        public IntPtr Camera_SetOrthographicSize;
        public IntPtr Camera_GetBackgroundColor;
        public IntPtr Camera_SetBackgroundColor;
        public IntPtr Camera_GetPrimary;
        public IntPtr Camera_SetPrimary;

        public IntPtr SceneManager_GetActiveScenePath;
        public IntPtr Scene_InstantiatePrefab;

        public IntPtr Entity_GetParent;
        public IntPtr Entity_SetParent;
        public IntPtr Entity_GetChildCount;
        public IntPtr Entity_GetChildAt;
        public IntPtr Transform_GetWorldTranslation;
        public IntPtr Transform_SetWorldTranslation;
        public IntPtr Transform_GetWorldRotation;

        public IntPtr UIText_GetText;
        public IntPtr UIText_SetText;
        public IntPtr UIText_GetColor;
        public IntPtr UIText_SetColor;
        public IntPtr UIText_GetFontSize;
        public IntPtr UIText_SetFontSize;

        public IntPtr UIButton_GetInteractable;
        public IntPtr UIButton_SetInteractable;
        public IntPtr UIButton_GetIsPointerInside;
        public IntPtr UIButton_GetIsPressed;
        public IntPtr UIButton_GetWasClickedThisFrame;

        public IntPtr SoundPlayer_Play;
        public IntPtr SoundPlayer_Stop;
        public IntPtr SoundPlayer_Pause;
        public IntPtr SoundPlayer_Resume;
        public IntPtr SoundPlayer_PlayOneShot;
        public IntPtr SoundPlayer_GetVolume;
        public IntPtr SoundPlayer_SetVolume;
        public IntPtr SoundPlayer_GetMute;
        public IntPtr SoundPlayer_SetMute;
        public IntPtr SoundPlayer_GetLoop;
        public IntPtr SoundPlayer_SetLoop;
        public IntPtr SoundPlayer_GetPlayOnStart;
        public IntPtr SoundPlayer_SetPlayOnStart;
        public IntPtr SoundPlayer_GetSoundHandle;
        public IntPtr SoundPlayer_SetSoundHandle;

        public IntPtr FontAsset_GetDefaultHandle;
        public IntPtr FontAsset_PreloadCharacters;
        public IntPtr FontAsset_PreloadTextAsync;
        public IntPtr FontAsset_WaitForPendingGenerations;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastHit2D
    {
        public Vector2 Point;
        public Vector2 Normal;
        public float Distance;
        public ulong EntityID;
        [MarshalAs(UnmanagedType.I1)]
        public bool Hit;
    }
}