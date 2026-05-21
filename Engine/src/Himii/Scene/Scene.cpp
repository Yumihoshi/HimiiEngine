#include "Scene.h"
#include "Entity.h"
#include "Hepch.h"

#include "Components.h"
#include "Himii/Asset/AssetManager.h"
#include "Himii/Scene/SpriteRendererUtility.h"
#include "Himii/Scene/SpriteAnimationUtility.h"
#include "Himii/Project/Project.h"
#include "Himii/Renderer/Renderer2D.h"
#include "Himii/Renderer/Renderer3D.h"
#include "Himii/Renderer/RenderCommand.h"
#include "Himii/Scene/SpriteAnimation.h"
#include "Himii/Scene/TileSet.h"
#include "Himii/Scene/TileMapData.h"
#include "Himii/Scene/TileMapCoordinateUtility.h"
#include "Himii/Scene/TilemapColliderBuilder.h"
#include "Himii/Scene/ParticleEmitterAsset.h"
#include "Himii/Renderer/Texture.h"
#include "Himii/Particle/ParticleSystem.h"
#include "Himii/Scripting/ScriptEngine.h"
#include "ScriptableEntity.h"

#include <glm/glm.hpp>

#include "Entity.h"

namespace Himii
{
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
        if (b2World_IsValid(m_Box2DWorld))
        {
            b2DestroyWorld(m_Box2DWorld);
        }
    }

    static void *ToPtr(b2BodyId id)
    {
        return reinterpret_cast<void *>(*reinterpret_cast<uintptr_t *>(&id));
    }
    static b2BodyId ToBodyId(void *ptr)
    {
        uintptr_t val = reinterpret_cast<uintptr_t>(ptr);
        return *reinterpret_cast<b2BodyId *>(&val);
    }

    static void AttachBoxColliderToBody(b2BodyId bodyId,
                                        const BoxCollider2DComponent& boxCollider,
                                        const TransformComponent& transform)
    {
        if (!b2Body_IsValid(bodyId))
            return;

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.enableContactEvents = true;
        shapeDef.density = boxCollider.Density;
        shapeDef.material.friction = boxCollider.Friction;
        shapeDef.material.restitution = boxCollider.Restitution;
        shapeDef.material.rollingResistance = boxCollider.RestitutionThreshold;

        const float absoluteScaleX = std::abs(transform.Scale.x);
        const float absoluteScaleY = std::abs(transform.Scale.y);
        const float halfWidth = boxCollider.Size.x * absoluteScaleX * 0.5f;
        const float halfHeight = boxCollider.Size.y * absoluteScaleY * 0.5f;
        const b2Polygon polygon = b2MakeOffsetBox(
                halfWidth, halfHeight,
                {boxCollider.Offset.x * transform.Scale.x, boxCollider.Offset.y * transform.Scale.y},
                b2MakeRot(0.0f));

        b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    }

    static void AttachCircleColliderToBody(b2BodyId bodyId,
                                           const CircleCollider2DComponent& circleCollider,
                                           const TransformComponent& transform)
    {
        if (!b2Body_IsValid(bodyId))
            return;

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.enableContactEvents = true;
        shapeDef.density = circleCollider.Density;
        shapeDef.material.friction = circleCollider.Friction;
        shapeDef.material.restitution = circleCollider.Restitution;
        shapeDef.material.rollingResistance = circleCollider.RestitutionThreshold;

        b2Circle circle;
        circle.center = {
            circleCollider.Offset.x * transform.Scale.x,
            circleCollider.Offset.y * transform.Scale.y};
        const float absoluteScaleX = std::abs(transform.Scale.x);
        const float absoluteScaleY = std::abs(transform.Scale.y);
        const float maxScale = std::max(absoluteScaleX, absoluteScaleY);
        circle.radius = circleCollider.Radius * maxScale;

        b2CreateCircleShape(bodyId, &shapeDef, &circle);
    }

    static b2BodyId CreateStaticPhysicsBody(b2WorldId world,
                                            entt::entity entityHandle,
                                            const TransformComponent& transform)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = {transform.Position.x, transform.Position.y};
        bodyDef.rotation = b2MakeRot(transform.Rotation.z);
        bodyDef.userData = (void*)(uintptr_t)(uint32_t)entityHandle;
        return b2CreateBody(world, &bodyDef);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name)
    {
        Entity entity(m_Registry.create(), this);

        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        auto &tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;

        m_EntityMap[uuid] = entity;

        return entity;
    }

    Entity Scene::CreateEntity(const std::string &name)
    {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateUIEntity(const std::string &name)
    {
        return CreateUIEntityWithUUID(UUID(),name);
    }

    Entity Scene::CreateUIEntityWithUUID(UUID uuid, const std::string &name)
    {
        Entity entity(m_Registry.create(), this);

        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<UITransformComponent>();
        auto &tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;

        m_EntityMap[uuid] = entity;

        return entity;
    }

    void Scene::DestroyEntity(entt::entity e)
    {
        if (!m_Registry.valid(e))
            return;

        if (auto *pid = m_Registry.try_get<IDComponent>(e))
        {
            auto it = m_EntityMap.find(pid->ID);
            if (it != m_EntityMap.end())
                m_EntityMap.erase(it);
        }
        // 脚本析构
        if (NativeScriptComponent *nsc = m_Registry.try_get<NativeScriptComponent>(e))
        {
            if (nsc->Instance)
            {
                nsc->Instance->OnDestroy();
            }
            if (nsc->DestroyScript)
            {
                nsc->DestroyScript(nsc);
            }
        }
        m_Registry.destroy(e);
    }

    void Scene::ClearEntities()
    {
        std::vector<entt::entity> entityHandles;
        m_Registry.view<IDComponent>().each(
            [&](entt::entity entityHandle, IDComponent&)
            {
                entityHandles.push_back(entityHandle);
            });

        for (entt::entity entityHandle : entityHandles)
            DestroyEntity(entityHandle);
    }

    void Scene::OnRuntimeStart()
    {
        ScriptEngine::OnRuntimeStart(this);
        OnPhysics2DStart();

        {
            auto animationView = m_Registry.view<SpriteAnimationComponent>();
            for (auto entityHandle : animationView)
            {
                Entity entity = {entityHandle, this};
                ResetSpriteAnimationPlayback(entity.GetComponent<SpriteAnimationComponent>());
            }
        }
        // 3. 实例化所有拥有 ScriptComponent 的实体
        {
            auto view = m_Registry.view<ScriptComponent>();
            for (auto e: view)
            {
                Entity entity = {e, this};
                ScriptEngine::OnCreateEntity(entity); // <--- 新增这行调用
            }
        }
    }

    void Scene::OnSimulationStart()
    {
        OnPhysics2DStart();
    }

    void Scene::OnSimulationStop()
    {
        OnPhysics2DStop();
    }

    void Scene::OnRuntimeStop()
    {
        OnPhysics2DStop();
        ScriptEngine::OnRuntimeStop();
    }

    void Scene::OnUpdateEditor(Timestep ts, EditorCamera &camera)
    {
        UpdateSpriteAnimations(ts, true);
        RenderScene(camera);
        RenderUI((float)m_ViewportWidth, (float)m_ViewportHeight);
    }

    void Scene::UpdateSpriteAnimations(Timestep timestep, bool allowEditorPreview)
    {
        auto view = m_Registry.view<SpriteAnimationComponent, SpriteRendererComponent>();
        auto assetManager = Project::TryGetAssetManager();
        if (!assetManager)
            return;

        const float deltaTimeSeconds = timestep.GetSeconds();

        for (auto entityHandle : view)
        {
            auto [animationComponent, spriteRendererComponent] =
                    view.get<SpriteAnimationComponent, SpriteRendererComponent>(entityHandle);

            if (animationComponent.AnimationHandle == 0
                || !assetManager->IsAssetHandleValid(animationComponent.AnimationHandle))
                continue;

            Ref<SpriteAnimation> animation = std::static_pointer_cast<SpriteAnimation>(
                    assetManager->GetAsset(animationComponent.AnimationHandle));
            if (!animation)
                continue;

            if (animationComponent.CurrentAnimationName.empty()
                || !animation->HasAnimation(animationComponent.CurrentAnimationName))
            {
                if (const SpriteAnimationClip* primaryClip = animation->GetPrimaryClip())
                    animationComponent.CurrentAnimationName = primaryClip->Name;
            }

            if (animation->GetFrameCount(animationComponent.CurrentAnimationName) == 0)
                continue;

            const bool shouldAdvance =
                    animationComponent.Playing
                    || (allowEditorPreview && animationComponent.PreviewInScene);

            if (shouldAdvance)
                AdvanceSpriteAnimation(animationComponent, *animation, deltaTimeSeconds);
        }
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
        {
            auto view = m_Registry.view<ScriptComponent>();
            for (auto e: view)
            {
                Entity entity = {e, this};
                ScriptEngine::OnUpdateScript(entity, ts);
            }

            m_Registry.view<NativeScriptComponent>().each(
                    [=](auto entity, auto &nsc)
                    {
                        // TODO: Move to Scene::OnScenePlay
                        if (!nsc.Instance)
                        {
                            nsc.Instance = nsc.InstantiateScript();
                            nsc.Instance->m_Entity = Entity{entity, this};
                            nsc.Instance->OnCreate();
                        }

                        nsc.Instance->OnUpdate(ts);
                    });
        }

        UpdateSpriteAnimations(ts, false);

        // Box2D 物理更新
        {
            const int32_t subStepCount = 2;
            b2World_Step(m_Box2DWorld, ts, subStepCount);
            ProcessPhysics2DContacts();

            auto view = m_Registry.view<Rigidbody2DComponent>();
            for (auto e: view)
            {
                Entity entity = {e, this};
                auto &transform = entity.GetComponent<TransformComponent>();
                auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();

                if (rb2d.RuntimeBody)
                {
                    b2BodyId bodyId = ToBodyId(rb2d.RuntimeBody);
                    if (b2Body_IsValid(bodyId))
                    {
                        b2Vec2 position = b2Body_GetPosition(bodyId);
                        transform.Position.x = position.x;
                        transform.Position.y = position.y;

                        b2Rot rotation = b2Body_GetRotation(bodyId);
                        transform.Rotation.z = b2Rot_GetAngle(rotation);
                    }
                }
            }
        }

        // 粒子发射器：按资源与 Transform 发射，再统一更新粒子系统
        if (Project::GetActive())
        {
            auto assetManager = Project::GetAssetManager();
            if (assetManager)
            {
                auto view = m_Registry.group<TransformComponent, ParticleEmitterComponent>();
                for (auto e : view)
                {
                    auto [transform, emitter] = view.get<TransformComponent, ParticleEmitterComponent>(e);
                    if (emitter.EmitterHandle == 0) continue;

                    Ref<Asset> assetRef = assetManager->GetAsset(emitter.EmitterHandle);
                    if (!assetRef) continue;

                    auto emitterAsset = std::static_pointer_cast<ParticleEmitterAsset>(assetRef);
                    if (!emitterAsset) continue;

                    emitter.EmissionAccumulator += ts * emitterAsset->EmissionRate;
                    int emitCount = static_cast<int>(std::floor(emitter.EmissionAccumulator));
                    if (emitCount <= 0) continue;

                    emitter.EmissionAccumulator -= static_cast<float>(emitCount);
                    ParticleProps props = emitterAsset->TemplateProps;
                    props.position = glm::vec3(transform.Position.x, transform.Position.y, 0.0f);

                    for (int i = 0; i < emitCount; ++i)
                        m_ParticleSystem.Emit(props);
                }
            }
        }
        m_ParticleSystem.OnUpdate(ts);

        Camera *mainCamera = nullptr;
        glm::mat4 cameraTransform{1.0f};
        {
            auto view = m_Registry.view<TransformComponent, CameraComponent>();
            view.each(
                    [&](entt::entity entity, TransformComponent &transform, CameraComponent &camera)
                    {
                        if (camera.Primary)
                        {
                            mainCamera = &camera.Camera;
                            cameraTransform = transform.GetTransform();
                        }
                    });
        }

        if (mainCamera)
        {
            Renderer2D::BeginScene(*mainCamera, cameraTransform);
            {
                auto assetManager = Project::TryGetAssetManager();
                auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
                view.each([&](entt::entity entity, TransformComponent &transform, SpriteRendererComponent &sprite)
                          {
                              if (!assetManager)
                                  return;

                              Entity sceneEntity = {entity, this};
                              const SpriteResolved resolved =
                                  ResolveSpriteRendererDrawable(sceneEntity, sprite, assetManager.get());
                              Himii::Renderer2D::DrawSprite(transform.GetTransform(), sprite, resolved, (int)entity);
                          });
            }
            {
                if (Project::GetActive())
                {
                    auto assetManager = Project::GetAssetManager();
                    auto view = m_Registry.view<TransformComponent, TilemapComponent>();
                    view.each([&](entt::entity entity, TransformComponent &transform, TilemapComponent &tilemap)
                    {
                        if (!assetManager || tilemap.TileMapHandle == 0) return;

                        auto mapAsset = assetManager->GetAsset(tilemap.TileMapHandle);
                        if (!mapAsset) return;
                        auto mapData = std::static_pointer_cast<TileMapData>(mapAsset);

                        Ref<TileSet> tileSet = nullptr;
                        if (mapData->GetTileSetHandle() != 0)
                        {
                            auto tsAsset = assetManager->GetAsset(mapData->GetTileSetHandle());
                            if (tsAsset)
                                tileSet = std::static_pointer_cast<TileSet>(tsAsset);
                        }

                        Himii::Renderer2D::DrawTilemap(transform.GetTransform(), mapData, tileSet, (int)entity);
                    });
                }
            }
            {
                auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
                view.each(
                        [&](entt::entity entity, TransformComponent &transform, CircleRendererComponent &circle) {
                            Himii::Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness,
                                                          circle.Fade, (int)entity);
                        });
            }
            // 粒子系统 2D 渲染（与 Renderer2D 同批次）
            auto particleAssetManager = Project::GetActive() ? Project::GetAssetManager() : nullptr;
            m_ParticleSystem.ForEachAlive([&](const ParticleSystem::ParticleView& p)
            {
                float t = 1.0f - p.remainingLife / p.lifetime;
                glm::vec4 color = glm::mix(p.colorBegin, p.colorEnd, t);
                float size = glm::mix(p.sizeBegin, p.sizeEnd, t);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), p.position)
                    * glm::rotate(glm::mat4(1.0f), p.rotation, glm::vec3(0, 0, 1))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(size));

                Ref<Texture2D> tex;
                if (p.textureHandle != 0 && particleAssetManager && particleAssetManager->IsAssetHandleValid(static_cast<AssetHandle>(p.textureHandle)))
                {
                    Ref<Asset> a = particleAssetManager->GetAsset(static_cast<AssetHandle>(p.textureHandle));
                    tex = std::dynamic_pointer_cast<Texture2D>(a);
                }

                if (p.shape == ParticleShape::Circle)
                    Renderer2D::DrawCircle(transform, color, 1.0f, 0.0025f);
                else
                {
                    if (tex)
                        Renderer2D::DrawQuad(transform, tex, 1.0f, color);
                    else
                        Renderer2D::DrawQuad(transform, color);
                }
            });
            Renderer2D::EndScene();
        }

        {
             // 3D Rendering (MeshComponent)
             if (mainCamera)
             {
                 Renderer3D::BeginScene(*mainCamera, cameraTransform);

                 bool is2D = false;
                 if (Project::GetActive())
                    is2D = Project::GetActive()->GetConfig().Is2D;

                 if (m_SkyboxTexture && !is2D)
                     Renderer3D::DrawSkybox(m_SkyboxTexture, *mainCamera, cameraTransform);

                 // Renderer3D::DrawGrid(*mainCamera, cameraTransform, is2D);

                 auto view = m_Registry.view<TransformComponent, MeshComponent>();
                 view.each([&](entt::entity entity, TransformComponent &transform, MeshComponent &mesh)
                 {
                     if (mesh.Type == MeshComponent::MeshType::Cube)
                         Renderer3D::DrawCube(transform.GetTransform(), mesh.Color, (int)entity);
                     else if (mesh.Type == MeshComponent::MeshType::Plane)
                         Renderer3D::DrawPlane(transform.GetTransform(), mesh.Color, (int)entity);
                     else if (mesh.Type == MeshComponent::MeshType::Sphere)
                         Renderer3D::DrawSphere(transform.GetTransform(), mesh.Color, (int)entity);
                     else if (mesh.Type == MeshComponent::MeshType::Capsule)
                         Renderer3D::DrawCapsule(transform.GetTransform(), mesh.Color, (int)entity);
                 });
                 Renderer3D::EndScene();
             }
        }
        RenderUI((float)m_ViewportWidth, (float)m_ViewportHeight);
    }

    static float RayCastCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
    {
        auto* ctx = (std::pair<Scene*, Scene::RaycastHit2D*>*)context;
        Scene* scene = ctx->first;
        Scene::RaycastHit2D* hit = ctx->second;
        
        hit->Hit = true;
        hit->Point = {point.x, point.y};
        hit->Normal = {normal.x, normal.y};
        hit->Distance = fraction;

        b2BodyId bodyId = b2Shape_GetBody(shapeId);
        void* userData = b2Body_GetUserData(bodyId);
        uint32_t entityHandle = (uint32_t)(uintptr_t)userData;
        
        entt::entity e = (entt::entity)entityHandle;
        if(scene->Registry().valid(e))
        {
             hit->EntityID = scene->Registry().get<IDComponent>(e).ID;
        }

        return fraction;
    }

    Scene::RaycastHit2D Scene::Raycast2D(glm::vec2 start, glm::vec2 end)
    {
        RaycastHit2D hit;
        hit.Hit = false;
        
        // Check if world is valid (Runtime running?)
        if (!b2World_IsValid(m_Box2DWorld)) return hit;

        b2Vec2 origin = {start.x, start.y};
        b2Vec2 translation = {end.x - start.x, end.y - start.y};
        b2QueryFilter filter = b2DefaultQueryFilter();
        
        std::pair<Scene*, RaycastHit2D*> context = {this, &hit};

        b2World_CastRay(m_Box2DWorld, origin, translation, filter, RayCastCallback, &context);
        
        if(hit.Hit) {
            float length = glm::length(end - start);
            hit.Distance *= length;
        }

        return hit;
    }

    void Scene::OnUpdateSimulation(Timestep ts, EditorCamera &camera)
    {
        // Box2D 物理更新
        {
            const int32_t subStepCount = 2;
            b2World_Step(m_Box2DWorld, ts, subStepCount);
            ProcessPhysics2DContacts();

            auto view = m_Registry.view<Rigidbody2DComponent>();
            for (auto e: view)
            {
                Entity entity = {e, this};
                auto &transform = entity.GetComponent<TransformComponent>();
                auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();

                if (rb2d.RuntimeBody)
                {
                    b2BodyId bodyId = ToBodyId(rb2d.RuntimeBody);
                    if (b2Body_IsValid(bodyId))
                    {
                        b2Vec2 position = b2Body_GetPosition(bodyId);
                        transform.Position.x = position.x;
                        transform.Position.y = position.y;

                        b2Rot rotation = b2Body_GetRotation(bodyId);
                        transform.Rotation.z = b2Rot_GetAngle(rotation);
                    }
                }
            }
        }

        UpdateSpriteAnimations(ts, false);
        RenderScene(camera);
    }

    template<typename Component>
    static void CopyComponent(entt::registry &dst, entt::registry &src,
                              const std::unordered_map<UUID, entt::entity> &enttMap)
    {
        auto view = src.view<Component>();
        for (auto e: view)
        {
            UUID uuid = src.get<IDComponent>(e).ID;
            // Find target entity by UUID
            if (enttMap.find(uuid) == enttMap.end())
                continue;

            entt::entity dstEnttID = enttMap.at(uuid);
            auto &component = src.get<Component>(e);
            dst.emplace_or_replace<Component>(dstEnttID, component);
        }
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> newScene = CreateRef<Scene>();

        newScene->m_ViewportWidth = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;

        newScene->m_SkyboxTexture = other->m_SkyboxTexture;

        auto &srcSceneRegistry = other->m_Registry;
        auto &dstSceneRegistry = newScene->m_Registry;
        std::unordered_map<UUID, entt::entity> enttMap;

        // Create entities in new scene
        auto idView = srcSceneRegistry.view<IDComponent>();
        for (auto e: idView)
        {
            UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
            const auto &name = srcSceneRegistry.get<TagComponent>(e).Tag;
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            enttMap[uuid] = (entt::entity)newEntity;
        }

        // Copy components
        CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<ScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<SpriteAnimationComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<MeshComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<TilemapComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<TilemapCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<ParticleEmitterComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        //UI
        CopyComponent<UITransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<UIImageComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<UITextComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

        return newScene;
    }

    Entity Scene::DuplicateEntity(Entity entity)
    {
        std::string name = entity.GetName();
        Entity newEntity = CreateEntity(name);

        if (entity.HasComponent<TransformComponent>())
            newEntity.GetComponent<TransformComponent>() = entity.GetComponent<TransformComponent>();

        if (entity.HasComponent<SpriteRendererComponent>())
            newEntity.AddComponent<SpriteRendererComponent>(entity.GetComponent<SpriteRendererComponent>());

        if (entity.HasComponent<CircleRendererComponent>())
            newEntity.AddComponent<CircleRendererComponent>(entity.GetComponent<CircleRendererComponent>());

        if (entity.HasComponent<CameraComponent>())
            newEntity.AddComponent<CameraComponent>(entity.GetComponent<CameraComponent>());

        if (entity.HasComponent<NativeScriptComponent>())
            newEntity.AddComponent<NativeScriptComponent>(entity.GetComponent<NativeScriptComponent>());

        if (entity.HasComponent<ScriptComponent>())
            newEntity.AddComponent<ScriptComponent>(entity.GetComponent<ScriptComponent>());

        if (entity.HasComponent<Rigidbody2DComponent>())
            newEntity.AddComponent<Rigidbody2DComponent>(entity.GetComponent<Rigidbody2DComponent>());

        if (entity.HasComponent<BoxCollider2DComponent>())
            newEntity.AddComponent<BoxCollider2DComponent>(entity.GetComponent<BoxCollider2DComponent>());

        if (entity.HasComponent<CircleCollider2DComponent>())
            newEntity.AddComponent<CircleCollider2DComponent>(entity.GetComponent<CircleCollider2DComponent>());

        if (entity.HasComponent<SpriteAnimationComponent>())
            newEntity.AddComponent<SpriteAnimationComponent>(entity.GetComponent<SpriteAnimationComponent>());

        if (entity.HasComponent<MeshComponent>())
            newEntity.AddComponent<MeshComponent>(entity.GetComponent<MeshComponent>());

        if (entity.HasComponent<TilemapComponent>())
            newEntity.AddComponent<TilemapComponent>(entity.GetComponent<TilemapComponent>());
        if (entity.HasComponent<TilemapCollider2DComponent>())
            newEntity.AddComponent<TilemapCollider2DComponent>(
                    entity.GetComponent<TilemapCollider2DComponent>());
        if (entity.HasComponent<ParticleEmitterComponent>())
            newEntity.AddComponent<ParticleEmitterComponent>(entity.GetComponent<ParticleEmitterComponent>());

        //UI
        if (entity.HasComponent<UITransformComponent>())
            newEntity.AddComponent<UITransformComponent>(entity.GetComponent<UITransformComponent>());
        if (entity.HasComponent<UIImageComponent>())
            newEntity.AddComponent<UIImageComponent>(entity.GetComponent<UIImageComponent>());
        if (entity.HasComponent<UITextComponent>())
            newEntity.AddComponent<UITextComponent>(entity.GetComponent<UITextComponent>());

        return newEntity;
    }

    Entity Scene::FindEntityByName(const std::string &name)
    {
        auto view = m_Registry.view<TagComponent>();
        for (auto entity: view)
        {
            const TagComponent &tc = view.get<TagComponent>(entity);
            if (tc.Tag == name)
                return Entity{entity, this};
        }
        return {};
    }

    Entity Scene::GetEntityByUUID(UUID uuid)
    {
        // TODO(Yan): Maybe should be assert
        if (m_EntityMap.find(uuid) != m_EntityMap.end())
            return {m_EntityMap.at(uuid), this};

        return {};
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        if (m_ViewportWidth == width && m_ViewportHeight == height)
            return;

        m_ViewportWidth = width;
        m_ViewportHeight = height;

        auto view = m_Registry.view<CameraComponent>();
        for (auto entity: view)
        {
            auto &cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
                cameraComponent.Camera.SetViewportSize(width, height);
        }
    }

    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity: view)
        {
            const auto &cameraComponent = view.get<CameraComponent>(entity);
            if (cameraComponent.Primary)
            {
                return Entity{entity, this};
            }
        }
        return {};
    }

    void Scene::OnPhysics2DStart()
    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2{0.0f, -9.8f};
        m_Box2DWorld = b2CreateWorld(&worldDef);

        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e: view)
        {
            Entity entity = {e, this};
            auto &transform = entity.GetComponent<TransformComponent>();
            auto &rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();

            b2BodyDef bodyDef = b2DefaultBodyDef();

            switch (rigidbody2D.Type)
            {
                case Rigidbody2DComponent::BodyType::Static:
                    bodyDef.type = b2BodyType::b2_staticBody;
                    break;
                case Rigidbody2DComponent::BodyType::Dynamic:
                    bodyDef.type = b2BodyType::b2_dynamicBody;
                    break;
                case Rigidbody2DComponent::BodyType::Kinematic:
                    bodyDef.type = b2BodyType::b2_kinematicBody;
                    break;
            }

            bodyDef.position = {transform.Position.x, transform.Position.y};
            bodyDef.rotation = b2MakeRot(transform.Rotation.z);
            bodyDef.fixedRotation = rigidbody2D.FixedRotation;
            bodyDef.userData = (void *)(uintptr_t)(uint32_t)entity; // 存储 Entity ID

            b2BodyId bodyId = b2CreateBody(m_Box2DWorld, &bodyDef);
            rigidbody2D.RuntimeBody = ToPtr(bodyId);

            if (entity.HasComponent<BoxCollider2DComponent>())
                AttachBoxColliderToBody(bodyId, entity.GetComponent<BoxCollider2DComponent>(), transform);

            if (entity.HasComponent<CircleCollider2DComponent>())
                AttachCircleColliderToBody(bodyId, entity.GetComponent<CircleCollider2DComponent>(), transform);
        }

        {
            auto colliderOnlyView = m_Registry.view<TransformComponent>();
            for (auto entityHandle : colliderOnlyView)
            {
                if (m_Registry.any_of<Rigidbody2DComponent>(entityHandle))
                    continue;

                const bool hasBoxCollider = m_Registry.all_of<BoxCollider2DComponent>(entityHandle);
                const bool hasCircleCollider =
                        m_Registry.all_of<CircleCollider2DComponent>(entityHandle);
                if (!hasBoxCollider && !hasCircleCollider)
                    continue;

                Entity entity = {entityHandle, this};
                const TransformComponent& transform = entity.GetComponent<TransformComponent>();
                const b2BodyId staticBodyId =
                        CreateStaticPhysicsBody(m_Box2DWorld, entityHandle, transform);

                if (hasBoxCollider)
                    AttachBoxColliderToBody(
                            staticBodyId, entity.GetComponent<BoxCollider2DComponent>(), transform);
                if (hasCircleCollider)
                    AttachCircleColliderToBody(
                            staticBodyId, entity.GetComponent<CircleCollider2DComponent>(), transform);
            }
        }

        auto tilemapColliderView =
                m_Registry.view<TransformComponent, TilemapComponent, TilemapCollider2DComponent>();
        for (auto entityHandle : tilemapColliderView)
        {
            Entity entity = {entityHandle, this};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& tilemap = entity.GetComponent<TilemapComponent>();
            auto& tilemapCollider = entity.GetComponent<TilemapCollider2DComponent>();

            if (!tilemapCollider.Enabled || tilemap.TileMapHandle == 0)
                continue;

            auto assetManager = Project::GetAssetManager();
            if (!assetManager)
                continue;

            Ref<TileMapData> mapData = std::static_pointer_cast<TileMapData>(
                    assetManager->GetAsset(tilemap.TileMapHandle));
            if (!mapData)
                continue;

            if (mapData->GetCellSize() <= 0.0f)
                continue;

            Ref<TileSet> tileSet;
            if (mapData->GetTileSetHandle() != 0)
            {
                tileSet = std::static_pointer_cast<TileSet>(
                        assetManager->GetAsset(mapData->GetTileSetHandle()));
            }

            if (!tileSet)
            {
                HIMII_CORE_WARNING(
                        "TilemapCollider2D: entity '{0}' has no TileSet; colliders were not created.",
                        entity.GetName());
                continue;
            }

            void* bodyUserData = (void*)(uintptr_t)(uint32_t)entity;
            const TilemapColliderBuildReport report = TilemapColliderBuilder::CreateColliderShapes(
                    m_Box2DWorld,
                    transform,
                    *mapData,
                    *tileSet,
                    bodyUserData,
                    tilemapCollider.MergeAdjacentCells);

            TilemapColliderBuilder::LogBuildReport(entity.GetName(), report);
        }
    }

    void Scene::OnPhysics2DStop()
    {
        if (b2World_IsValid(m_Box2DWorld))
        {
            b2DestroyWorld(m_Box2DWorld);
            m_Box2DWorld = {0}; // Reset ID
        }
    }

    Entity Scene::GetEntityFromShape(b2ShapeId shapeId)
    {
        if (!b2Shape_IsValid(shapeId))
            return {};

        b2BodyId bodyId = b2Shape_GetBody(shapeId);
        if (!b2Body_IsValid(bodyId))
            return {};

        void *userData = b2Body_GetUserData(bodyId);
        if (!userData)
            return {};

        entt::entity handle = static_cast<entt::entity>(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(userData)));
        if (!m_Registry.valid(handle))
            return {};

        return Entity{handle, this};
    }

    void Scene::ProcessPhysics2DContacts()
    {
        if (!b2World_IsValid(m_Box2DWorld))
            return;

        b2ContactEvents events = b2World_GetContactEvents(m_Box2DWorld);

        for (int i = 0; i < events.beginCount; ++i)
        {
            const b2ContactBeginTouchEvent &ev = events.beginEvents[i];
            Entity entityA = GetEntityFromShape(ev.shapeIdA);
            Entity entityB = GetEntityFromShape(ev.shapeIdB);
            if (entityA && entityB)
            {
                ScriptEngine::OnCollisionEnter2D(entityA, entityB);
                ScriptEngine::OnCollisionEnter2D(entityB, entityA);
            }
        }

        for (int i = 0; i < events.endCount; ++i)
        {
            const b2ContactEndTouchEvent &ev = events.endEvents[i];
            if (!b2Shape_IsValid(ev.shapeIdA) || !b2Shape_IsValid(ev.shapeIdB))
                continue;

            Entity entityA = GetEntityFromShape(ev.shapeIdA);
            Entity entityB = GetEntityFromShape(ev.shapeIdB);
            if (entityA && entityB)
            {
                ScriptEngine::OnCollisionExit2D(entityA, entityB);
                ScriptEngine::OnCollisionExit2D(entityB, entityA);
            }
        }
    }

    void Scene::RenderScene(EditorCamera &camera)
    {
        Renderer2D::BeginScene(camera);

        // Draw Sprites
        {
            auto assetManager = Project::TryGetAssetManager();
            auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
            view.each([&](entt::entity entity, TransformComponent &transform, SpriteRendererComponent &sprite)
                      {
                          if (!assetManager)
                              return;

                          Entity sceneEntity = {entity, this};
                          const SpriteResolved resolved =
                              ResolveSpriteRendererDrawable(sceneEntity, sprite, assetManager.get());
                          Himii::Renderer2D::DrawSprite(transform.GetTransform(), sprite, resolved, (int)entity);
                      });
        }
        
        // Draw Tilemaps
        {
            if (Project::GetActive())
            {
                auto assetManager = Project::GetAssetManager();
                auto view = m_Registry.view<TransformComponent, TilemapComponent>();
                view.each([&](entt::entity entity, TransformComponent &transform, TilemapComponent &tilemap)
                {
                    if (!assetManager || tilemap.TileMapHandle == 0) return;

                    auto mapAsset = assetManager->GetAsset(tilemap.TileMapHandle);
                    if (!mapAsset) return;
                    auto mapData = std::static_pointer_cast<TileMapData>(mapAsset);

                    Ref<TileSet> tileSet = nullptr;
                    if (mapData->GetTileSetHandle() != 0)
                    {
                        auto tsAsset = assetManager->GetAsset(mapData->GetTileSetHandle());
                        if (tsAsset)
                            tileSet = std::static_pointer_cast<TileSet>(tsAsset);
                    }

                    Himii::Renderer2D::DrawTilemap(transform.GetTransform(), mapData, tileSet, (int)entity);
                });
            }
        }

        // Draw Circles
        {
            auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
            view.each(
                    [&](entt::entity entity, TransformComponent &transform, CircleRendererComponent &circle) {
                        Himii::Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness,
                                                      circle.Fade, (int)entity);
                    });
        }

        Renderer2D::EndScene();

        // Draw Meshes
        {
            Renderer3D::BeginScene(camera);

            bool is2D = false;
            if (Project::GetActive())
               is2D = Project::GetActive()->GetConfig().Is2D;

            if (m_SkyboxTexture && !is2D)
                Renderer3D::DrawSkybox(m_SkyboxTexture, camera);

            // Grid is handled by EditorLayer for 'Show Grid' toggle support.
            // Renderer3D::DrawGrid(camera, is2D);

            auto view = m_Registry.view<TransformComponent, MeshComponent>();
            view.each([&](entt::entity entity, TransformComponent &transform, MeshComponent &mesh)
            {
                if (mesh.Type == MeshComponent::MeshType::Cube)
                    Renderer3D::DrawCube(transform.GetTransform(), mesh.Color, (int)entity);
                else if (mesh.Type == MeshComponent::MeshType::Plane)
                    Renderer3D::DrawPlane(transform.GetTransform(), mesh.Color, (int)entity);
                else if (mesh.Type == MeshComponent::MeshType::Sphere)
                    Renderer3D::DrawSphere(transform.GetTransform(), mesh.Color, (int)entity);
                else if (mesh.Type == MeshComponent::MeshType::Capsule)
                    Renderer3D::DrawCapsule(transform.GetTransform(), mesh.Color, (int)entity);
            });
            Renderer3D::EndScene();
        }
    }

    void Scene::RenderUI(float viewportWidth, float viewportHeight)
    {
        if (viewportWidth == 0.0f || viewportHeight == 0.0f)
            return;

        glm::mat4 projection = glm::ortho(0.0f, viewportWidth, 0.0f, viewportHeight, -1.0f, 1.0f);
        glm::mat4 view = glm::mat4(1.0f);

        // 关闭深度测试，开启透明混合
        RenderCommand::SetDepthTest(false);

        Renderer2D::BeginScene(projection, view);

        // 3. 获取所有 UI 实体
        auto viewGrp = m_Registry.view<UITransformComponent,UIImageComponent>();
        viewGrp.each(
                [&](auto entity, auto &transform, auto &image)
                {
                    glm::mat4 transformMat = transform.GetTransform();

                    if (image.Texture)
                    {
                        Renderer2D::DrawQuad(transformMat, image.Texture, 1.0f, image.Color, (int)entity);
                    }
                    else
                    {
                        Renderer2D::DrawQuad(transformMat, image.Color, (int)entity);
                    }
                });
        auto textView = m_Registry.view<UITransformComponent, UITextComponent>();
        textView.each(
                [&](auto entity, auto &transform, auto &text)
                {
                    if (text.FontAsset)
                    {
                        Renderer2D::DrawString(text.TextString, text.FontAsset, transform.GetTransform(), text.Color,
                                               (int)entity);
                    }
                });

        Renderer2D::EndScene();

        // 4. 恢复深度测试
        RenderCommand::SetDepthTest(true);
    }

    //
    template<typename T>
    void Scene::OnComponentAdded(Entity emtity, T &component)
    {
    }
    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent &component)
    {
        if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
        {
            component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
        }
    }

    template<>
    void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<SpriteAnimationComponent>(Entity entity, SpriteAnimationComponent &component)
    {
        if (!entity.HasComponent<SpriteRendererComponent>())
            HIMII_CORE_WARNING("Entity '{0}' has Sprite Animation but no Sprite Renderer.",
                            entity.GetName());
    }

    template<>
    void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent &component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TilemapComponent>(Entity entity, TilemapComponent &component)
    {
    }
    template<>
    void Scene::OnComponentAdded<TilemapCollider2DComponent>(Entity entity,
                                                             TilemapCollider2DComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<ParticleEmitterComponent>(Entity entity, ParticleEmitterComponent &component)
    {
    }
    template<>
    void Scene::OnComponentAdded<UITransformComponent>(Entity entity, UITransformComponent &component)
    {
    }
    template<>
    void Scene::OnComponentAdded<UIImageComponent>(Entity entity, UIImageComponent &component)
    {
    }
    template<>
    void Scene::OnComponentAdded<UITextComponent>(Entity entity, UITextComponent &component)
    {
    }

} // namespace Himii
