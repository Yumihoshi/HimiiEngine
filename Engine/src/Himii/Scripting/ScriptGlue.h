#pragma once
#include "glm/glm.hpp"

namespace Himii
{

    struct ScriptEngineData {
        void *LogFunc;

        //Entity
        void *Entity_HasComponent;

        //Scene
        void *Scene_CreateEntity;
        void *Scene_DestroyEntity;
        void *Scene_FindEntityByName;

        // Transform
        void *Transform_GetTranslation;
        void *Transform_SetTranslation;
        void *Transform_GetRotation;
        void *Transform_SetRotation;
        void *Transform_GetScale;
        void *Transform_SetScale;

        // Input
        void *Input_IsKeyDown;
        void *Input_IsMouseButtonDown;
        void *Input_GetMousePosition;

        // Rigidbody2D
        void *Rigidbody2D_ApplyLinearImpulse;
        void *Rigidbody2D_ApplyLinearImpulseToCenter;
        void *Rigidbody2D_GetLinearVelocity;
        void *Rigidbody2D_SetLinearVelocity;

        // Tilemap
        void *Tilemap_GetSize;
        void *Tilemap_SetSize;
        void *Tilemap_GetTile;
        void *Tilemap_SetTile;

        // Physics2D
        void *Physics2D_Raycast;

        // Time
        void *Time_GetDeltaTime;

        // Scene
        void *SceneManager_LoadScene;

        // SpriteRenderer
        void *SpriteRenderer_GetColor;
        void *SpriteRenderer_SetColor;
        void *SpriteRenderer_GetSpriteHandle;
        void *SpriteRenderer_SetSpriteHandle;
        void *SpriteRenderer_GetTextureHandle;
        void *SpriteRenderer_SetTextureHandle;
        void *SpriteRenderer_GetFlipHorizontal;
        void *SpriteRenderer_SetFlipHorizontal;

        void *SpriteAnimation_GetPlaying;
        void *SpriteAnimation_SetPlaying;
        void *SpriteAnimation_GetFrameRate;
        void *SpriteAnimation_SetFrameRate;
        void *SpriteAnimation_ResetPlayback;
        void *SpriteAnimation_Play;
        void *SpriteAnimation_GetCurrentAnimationName;
        void *SpriteAnimation_SetCurrentAnimationName;

        void *Tilemap_GetBounds;
    };

    class ScriptGlue {
    public:
        //static void RegisterFunctions();
        static ScriptEngineData GetNativeFunctions();
    };

}
