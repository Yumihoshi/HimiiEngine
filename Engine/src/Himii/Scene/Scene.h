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

        void OnUpdateEditor(Timestep ts,EditorCamera &camera);
        void OnUpdateRuntime(Timestep ts);
        void OnUpdateSimulation(Timestep ts,EditorCamera &camera);

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

    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
        std::unordered_map<UUID, entt::entity> m_EntityMap;
        bool m_UseExternalVP{false};
        glm::mat4 m_ExternalVP{1.0f};

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
