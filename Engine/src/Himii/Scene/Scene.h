#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include "Himii/Core/Timestep.h"
#include "Himii/Core/UUID.h"
#include "Himii/Renderer/EditorCamera.h"
#include "Himii/Scene/SceneCamera.h"
#include "Himii/Renderer/Texture.h"

#include "box2d/box2d.h"
#include "Himii/Particle/ParticleSystem.h"

namespace Himii
{
    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        static Ref<Scene> Copy(Ref<Scene> other);
        
        struct RaycastHit2D {
            glm::vec2 Point;
            glm::vec2 Normal;
            float Distance;
            uint64_t EntityID; 
            bool Hit;
        };
        RaycastHit2D Raycast2D(glm::vec2 start, glm::vec2 end);

        Entity CreateEntityWithUUID(UUID uuid, const std::string &name);
        Entity CreateEntity(const std::string &name);

        Entity CreateUIEntity(const std::string &name);
        Entity CreateUIEntityWithUUID(UUID uuid, const std::string &name);
        Entity CreateCanvasEntity(const std::string &name = "Canvas");
        Entity CreateUIButtonEntity(const std::string &name = "Button");

        struct UserInterfacePointerFrameInput
        {
            /// false = 本帧不处理 UI 指针（编辑态 / 视口未悬停等）。
            bool Enabled = false;
            bool HasPosition = false;
            glm::vec2 PositionInTargetPixels{0.0f};
            bool PrimaryButtonHeld = false;
            bool PrimaryButtonPressedThisFrame = false;
            bool PrimaryButtonReleasedThisFrame = false;
        };

        void SetUserInterfacePointerInput(const UserInterfacePointerFrameInput &input);
        bool WasUserInterfacePointerHandled() const { return m_UserInterfacePointerHandled; }

        void DestroyEntity(entt::entity e);
        void ClearEntities();

        entt::registry &Registry()
        {
            return m_Registry;
        }

        void OnRuntimeStart();
        void OnSimulationStart();
        void RunScriptFixedUpdate(Timestep ts);
        void OnRuntimeStop();
        void OnSimulationStop();

        void OnUpdateEditor(Timestep ts, EditorCamera &camera, bool drawUserInterfaceContent = true);
        void OnUpdateRuntime(Timestep ts, bool drawUserInterfaceContent = true);
        void OnUpdateSimulation(Timestep ts,EditorCamera &camera);
        void RenderEditorView(EditorCamera& camera, bool drawUserInterfaceContent = true);
        /// 使用场景 Primary Camera 渲染世界与 Runtime UI；无有效相机时返回 false。
        bool RenderGameView(
                uint32_t targetWidth, uint32_t targetHeight,
                bool drawUserInterfaceContent = true);

        void UpdateSpriteAnimations(Timestep timestep, bool allowEditorPreview);

        void SetExternalViewProjection(const glm::mat4 *vp)
        {
            if (vp)
            {
                m_UseExternalVP = true;
                m_ExternalVP = *vp;
            }
            else
            {
                m_UseExternalVP = false;
            }
        }

        void OnViewportResize(uint32_t width, uint32_t hieght);

        Entity DuplicateEntity(Entity entity);

        Entity FindEntityByName(const std::string &name);
        Entity GetEntityByUUID(UUID uuid);
        Entity GetEntityByUUID(UUID uuid) const;

        Entity GetPrimaryCameraEntity();

        Entity GetParentEntity(Entity entity) const;
        const std::vector<UUID>& GetEntityChildren(Entity entity) const;
        std::vector<Entity> GetRootEntities(bool userInterfaceEntities) const;
        bool EntitiesShareTransformDomain(Entity left, Entity right) const;

        bool SetEntityParent(Entity child, Entity parent, bool keepWorldPosition = true);
        void UnparentEntity(Entity child, bool keepWorldPosition = true);
        bool IsEntityDescendantOf(Entity potentialDescendant, Entity potentialAncestor) const;

        glm::mat4 GetEntityWorldTransformMatrix(Entity entity) const;
        glm::vec3 GetEntityWorldTranslation(Entity entity) const;
        glm::vec3 GetEntityWorldRotation(Entity entity) const;
        glm::vec3 GetEntityWorldScale(Entity entity) const;
        void ApplyWorldMatrixAsLocalTransform(Entity entity, const glm::mat4& worldMatrix);
        void MarkEntityTransformDirty(Entity entity);
        void NotifyEntityLocalTransformChanged(Entity entity);
        void SyncEntityTransformSubtreeToPhysics(Entity entity);
        void RebuildHierarchyCache();

        Entity FindCanvasEntity() const;
        bool IsEntityUnderCanvas(Entity entity) const;

        struct CanvasLayoutContext
        {
            bool Valid = false;
            glm::vec2 TargetSize{0.0f};
            glm::vec2 LogicalSize{0.0f};
            float ScaleFactor = 1.0f;
        };

        struct ResolvedRectTransform
        {
            bool Valid = false;
            glm::vec2 Size{0.0f};
            glm::mat4 WorldTransform{1.0f};
        };

        CanvasLayoutContext GetCanvasLayoutContext(float targetWidth, float targetHeight) const;
        ResolvedRectTransform ResolveRectTransform(
                Entity entity, float targetWidth, float targetHeight) const;
        float ComputeCanvasScaleFactor(float targetWidth, float targetHeight) const;
        glm::mat4 GetCanvasToScreenMatrix(float viewportWidth, float viewportHeight) const;
        void SyncCanvasReferenceResolutionToTransform(Entity canvasEntity);

        struct OrthographicViewBounds {
            bool Valid = false;
            glm::mat4 CameraWorldMatrix{1.0f};
            float Left = 0.0f;
            float Right = 0.0f;
            float Bottom = 0.0f;
            float Top = 0.0f;
        };

        /// Edit：Canvas 设计框在世界 XY（z=0）上的半宽半高；中心固定在世界原点。
        struct DesignFrameWorldBounds {
            bool Valid = false;
            float HalfWidth = 0.0f;
            float HalfHeight = 0.0f;
            float DesignToWorldScale = 1.0f;
        };

        OrthographicViewBounds GetPrimaryOrthographicViewBounds() const;
        DesignFrameWorldBounds GetDesignFrameWorldBounds() const;
        /// 设计空间（中心原点）→ 世界空间（Edit 预览用，中心钉在世界原点）。
        glm::mat4 GetDesignToWorldMatrix() const;
        void DrawPrimaryCameraBounds(const glm::vec4& color) const;
        void DrawCanvasDesignBounds(const glm::vec4& color) const;
        void RenderUIInEditor(EditorCamera& editorCamera, bool drawUserInterfaceContent = true);
        bool IsWorldPositionInsideDesignFrame(const glm::vec2& worldPosition) const;

        /// 将 Transform 写入 Box2D 刚体（Play/Simulate 期间脚本改 Position/Rotation 时调用）。
        void SyncEntityTransformToPhysics(Entity entity);

        template<typename... Components>
        auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        void SetSkybox(const Ref<TextureCube> &skybox)
        {
            m_SkyboxTexture = skybox;
        }
        Ref<TextureCube> GetSkybox() const
        {
            return m_SkyboxTexture;
        }

    private:
        template<typename T>
        void OnComponentAdded(Entity entity, T &component);

        void OnPhysics2DStart();
        void OnPhysics2DStop();
        void ProcessPhysics2DContacts();
        Entity GetEntityFromShape(b2ShapeId shapeId);

        void RenderScene(EditorCamera &camera);

        void RenderUI(float viewportWidth, float viewportHeight);
        void PrepareUserInterfaceFonts();
        void RenderUIElements(
                const glm::mat4& designToTargetMatrix, float targetWidth, float targetHeight);
        void ProcessUserInterfacePointer();
        Entity HitTestUserInterfaceButton(
                float targetWidth, float targetHeight, const glm::vec2 &pointerInTargetPixels) const;
        bool IsPointInsideResolvedRect(
                const ResolvedRectTransform &resolvedRectTransform,
                const glm::mat4 &designToTargetMatrix,
                const glm::vec2 &pointerInTargetPixels) const;
        void ClearUserInterfacePointerTransientState();

    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
        std::unordered_map<UUID, entt::entity> m_EntityMap;
        bool m_UseExternalVP{false};
        glm::mat4 m_ExternalVP{1.0f};

        UserInterfacePointerFrameInput m_UserInterfacePointerInput{};
        bool m_UserInterfacePointerHandled = false;
        UUID m_UserInterfaceHoverEntityIdentifier = 0;
        UUID m_UserInterfacePressedEntityIdentifier = 0;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;

        Ref<TextureCube> m_SkyboxTexture;

        b2WorldId m_Box2DWorld;

        ParticleSystem m_ParticleSystem{ 50000 };

        std::unordered_map<UUID, std::vector<UUID>> m_ChildrenCache;
        static inline const std::vector<UUID> s_EmptyChildrenList{};

        void ApplyMatrixAsLocalTransform(Entity entity, const glm::mat4& localMatrix);
        void SetEntityWorldTransformFromPhysics(Entity entity, const glm::vec2& worldPosition, float worldRotationZ);
    };
}
