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
#include "Himii/Renderer/Font.h"
#include "Himii/Particle/ParticleSystem.h"
#include "Himii/Scripting/ScriptEngine.h"
#include "ScriptableEntity.h"
#include "Himii/Math/Math.h"
#include "Himii/Core/Log.h"

#include <glm/glm.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <functional>

#include "Entity.h"

namespace Himii
{
    struct SpriteDrawSortEntry
    {
        entt::entity EntityHandle{};
        TransformComponent* Transform = nullptr;
        SpriteRendererComponent* Sprite = nullptr;
    };

    static void DrawSpriteRenderersSorted(Scene* scene, entt::registry& registry, AssetManager* assetManager)
    {
        if (!assetManager)
            return;

        std::vector<SpriteDrawSortEntry> drawEntries;
        auto view = registry.view<TransformComponent, SpriteRendererComponent>();
        for (auto entityHandle : view)
        {
            drawEntries.push_back({
                entityHandle,
                &view.get<TransformComponent>(entityHandle),
                &view.get<SpriteRendererComponent>(entityHandle)
            });
        }

        std::stable_sort(drawEntries.begin(), drawEntries.end(),
                         [](const SpriteDrawSortEntry& left, const SpriteDrawSortEntry& right)
                         {
                             if (left.Sprite->SortingLayer != right.Sprite->SortingLayer)
                                 return left.Sprite->SortingLayer < right.Sprite->SortingLayer;
                             return left.Sprite->SortingOrder < right.Sprite->SortingOrder;
                         });

        for (const SpriteDrawSortEntry& entry : drawEntries)
        {
            Entity sceneEntity = {entry.EntityHandle, scene};
            const SpriteResolved resolved =
                ResolveSpriteRendererDrawable(sceneEntity, *entry.Sprite, assetManager);
            Renderer2D::DrawSprite(scene->GetEntityWorldTransformMatrix(sceneEntity), *entry.Sprite, resolved,
                                   static_cast<int>(entry.EntityHandle));
        }
    }

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

    static b2Filter BuildColliderShapeFilter(int layerIndex)
    {
        if (Project::GetActive())
            return Project::GetConfig().Physics2DLayers.BuildShapeFilter(layerIndex);

        b2Filter filter = b2DefaultFilter();
        filter.categoryBits = 1u;
        filter.maskBits = 0xFFFFFFFFu;
        return filter;
    }

    static void AttachBoxColliderToBody(b2BodyId bodyId,
                                        const BoxCollider2DComponent& boxCollider,
                                        const glm::vec3& worldScale)
    {
        if (!b2Body_IsValid(bodyId))
            return;

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.enableContactEvents = true;
        shapeDef.isSensor = boxCollider.IsTrigger;
        shapeDef.filter = BuildColliderShapeFilter(boxCollider.Layer);
        shapeDef.density = boxCollider.Density;
        shapeDef.material.friction = boxCollider.Friction;
        shapeDef.material.restitution = boxCollider.Restitution;
        shapeDef.material.rollingResistance = boxCollider.RestitutionThreshold;

        const float absoluteScaleX = std::abs(worldScale.x);
        const float absoluteScaleY = std::abs(worldScale.y);
        const float halfWidth = boxCollider.Size.x * absoluteScaleX * 0.5f;
        const float halfHeight = boxCollider.Size.y * absoluteScaleY * 0.5f;
        const b2Polygon polygon = b2MakeOffsetBox(
                halfWidth, halfHeight,
                {boxCollider.Offset.x * worldScale.x, boxCollider.Offset.y * worldScale.y},
                b2MakeRot(0.0f));

        b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    }

    static void AttachCircleColliderToBody(b2BodyId bodyId,
                                           const CircleCollider2DComponent& circleCollider,
                                           const glm::vec3& worldScale)
    {
        if (!b2Body_IsValid(bodyId))
            return;

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.enableContactEvents = true;
        shapeDef.isSensor = circleCollider.IsTrigger;
        shapeDef.filter = BuildColliderShapeFilter(circleCollider.Layer);
        shapeDef.density = circleCollider.Density;
        shapeDef.material.friction = circleCollider.Friction;
        shapeDef.material.restitution = circleCollider.Restitution;
        shapeDef.material.rollingResistance = circleCollider.RestitutionThreshold;

        b2Circle circle;
        circle.center = {
            circleCollider.Offset.x * worldScale.x,
            circleCollider.Offset.y * worldScale.y};
        const float absoluteScaleX = std::abs(worldScale.x);
        const float absoluteScaleY = std::abs(worldScale.y);
        const float maxScale = std::max(absoluteScaleX, absoluteScaleY);
        circle.radius = circleCollider.Radius * maxScale;

        b2CreateCircleShape(bodyId, &shapeDef, &circle);
    }

    static b2BodyId CreateStaticPhysicsBody(b2WorldId world, Scene* scene, Entity entity)
    {
        const glm::vec3 worldTranslation = scene->GetEntityWorldTranslation(entity);
        const glm::vec3 worldRotation = scene->GetEntityWorldRotation(entity);

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = {worldTranslation.x, worldTranslation.y};
        bodyDef.rotation = b2MakeRot(worldRotation.z);
        bodyDef.userData = (void*)(uintptr_t)(uint32_t)(entt::entity)entity;
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
        entity.AddComponent<RectTransformComponent>();
        auto &tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;

        m_EntityMap[uuid] = entity;

        return entity;
    }

    Entity Scene::CreateCanvasEntity(const std::string &name)
    {
        if (FindCanvasEntity())
        {
            HIMII_CORE_WARNING("Scene already has a Canvas; only one Canvas is allowed.");
            return {};
        }

        Entity canvasEntity = CreateUIEntity(name.empty() ? "Canvas" : name);
        auto& canvas = canvasEntity.AddComponent<CanvasComponent>();
        canvas.ReferenceResolution = {1920.0f, 1080.0f};
        canvas.MatchWidthOrHeight = 0.5f;
        SyncCanvasReferenceResolutionToTransform(canvasEntity);
        return canvasEntity;
    }

    Entity Scene::CreateUIButtonEntity(const std::string &name)
    {
        Entity buttonEntity = CreateUIEntity(name.empty() ? "Button" : name);
        auto& userInterfaceTransform = buttonEntity.GetComponent<RectTransformComponent>();
        userInterfaceTransform.SizeDelta = {160.0f, 40.0f};
        userInterfaceTransform.ResolvedSize = userInterfaceTransform.SizeDelta;

        auto& image = buttonEntity.AddComponent<UIImageComponent>();
        image.Color = {1.0f, 1.0f, 1.0f, 1.0f};
        buttonEntity.AddComponent<UIButtonComponent>();
        return buttonEntity;
    }

    void Scene::SetUserInterfacePointerInput(const UserInterfacePointerFrameInput &input)
    {
        m_UserInterfacePointerInput = input;
    }

    void Scene::ClearUserInterfacePointerTransientState()
    {
        auto buttonView = m_Registry.view<UIButtonComponent>();
        for (entt::entity entityHandle : buttonView)
        {
            auto& button = buttonView.get<UIButtonComponent>(entityHandle);
            button.WasClickedThisFrame = false;
        }
    }

    bool Scene::IsPointInsideResolvedRect(
            const ResolvedRectTransform &resolvedRectTransform,
            const glm::mat4 &designToTargetMatrix,
            const glm::vec2 &pointerInTargetPixels) const
    {
        if (!resolvedRectTransform.Valid)
            return false;

        const glm::mat4 drawTransform = designToTargetMatrix
                * resolvedRectTransform.WorldTransform
                * glm::scale(glm::mat4(1.0f), glm::vec3(resolvedRectTransform.Size, 1.0f));
        const glm::mat4 inverseDrawTransform = glm::inverse(drawTransform);
        const glm::vec4 localPoint = inverseDrawTransform
                * glm::vec4(pointerInTargetPixels.x, pointerInTargetPixels.y, 0.0f, 1.0f);
        return std::abs(localPoint.x) <= 0.5f && std::abs(localPoint.y) <= 0.5f;
    }

    Entity Scene::HitTestUserInterfaceButton(
            float targetWidth, float targetHeight, const glm::vec2 &pointerInTargetPixels) const
    {
        Entity canvasEntity = FindCanvasEntity();
        if (!canvasEntity)
            return {};

        const glm::mat4 designToTargetMatrix = GetCanvasToScreenMatrix(targetWidth, targetHeight);
        std::vector<Entity> buttonsInDrawOrder;

        std::function<void(Entity)> collectSubtree;
        collectSubtree = [&](Entity entity)
        {
            if (!entity || !entity.HasComponent<RectTransformComponent>())
                return;

            if (entity.HasComponent<UIButtonComponent>()
                && entity.GetComponent<UIButtonComponent>().Interactable)
            {
                buttonsInDrawOrder.push_back(entity);
            }

            for (UUID childIdentifier : GetEntityChildren(entity))
                collectSubtree(GetEntityByUUID(childIdentifier));
        };

        for (UUID childIdentifier : GetEntityChildren(canvasEntity))
            collectSubtree(GetEntityByUUID(childIdentifier));

        for (auto iterator = buttonsInDrawOrder.rbegin(); iterator != buttonsInDrawOrder.rend();
             ++iterator)
        {
            Entity entity = *iterator;
            const ResolvedRectTransform resolvedRectTransform =
                    ResolveRectTransform(entity, targetWidth, targetHeight);
            if (IsPointInsideResolvedRect(
                        resolvedRectTransform, designToTargetMatrix, pointerInTargetPixels))
                return entity;
        }

        return {};
    }

    void Scene::ProcessUserInterfacePointer()
    {
        m_UserInterfacePointerHandled = false;
        ClearUserInterfacePointerTransientState();

        const float targetWidth = static_cast<float>(m_ViewportWidth);
        const float targetHeight = static_cast<float>(m_ViewportHeight);

        if (!m_UserInterfacePointerInput.Enabled || targetWidth <= 0.0f || targetHeight <= 0.0f)
        {
            if (m_UserInterfaceHoverEntityIdentifier != 0)
            {
                Entity previousHover = GetEntityByUUID(m_UserInterfaceHoverEntityIdentifier);
                if (previousHover && previousHover.HasComponent<UIButtonComponent>())
                {
                    previousHover.GetComponent<UIButtonComponent>().IsPointerInside = false;
                    ScriptEngine::OnPointerEvent(previousHover, ScriptEngine::ScriptPointerEventType::Exit);
                }
                m_UserInterfaceHoverEntityIdentifier = 0;
            }
            if (m_UserInterfacePressedEntityIdentifier != 0)
            {
                Entity previousPressed = GetEntityByUUID(m_UserInterfacePressedEntityIdentifier);
                if (previousPressed && previousPressed.HasComponent<UIButtonComponent>())
                {
                    previousPressed.GetComponent<UIButtonComponent>().IsPressed = false;
                    ScriptEngine::OnPointerEvent(previousPressed, ScriptEngine::ScriptPointerEventType::Up);
                }
                m_UserInterfacePressedEntityIdentifier = 0;
            }
            return;
        }

        Entity hitEntity{};
        if (m_UserInterfacePointerInput.HasPosition)
        {
            hitEntity = HitTestUserInterfaceButton(
                    targetWidth, targetHeight, m_UserInterfacePointerInput.PositionInTargetPixels);
        }

        const UUID hitIdentifier = hitEntity ? hitEntity.GetUUID() : UUID(0);
        if (hitEntity)
            m_UserInterfacePointerHandled = true;

        // Hover enter/exit
        if (hitIdentifier != m_UserInterfaceHoverEntityIdentifier)
        {
            if (m_UserInterfaceHoverEntityIdentifier != 0)
            {
                Entity previousHover = GetEntityByUUID(m_UserInterfaceHoverEntityIdentifier);
                if (previousHover && previousHover.HasComponent<UIButtonComponent>())
                {
                    previousHover.GetComponent<UIButtonComponent>().IsPointerInside = false;
                    ScriptEngine::OnPointerEvent(previousHover, ScriptEngine::ScriptPointerEventType::Exit);
                }
            }
            m_UserInterfaceHoverEntityIdentifier = hitIdentifier;
            if (hitEntity && hitEntity.HasComponent<UIButtonComponent>())
            {
                hitEntity.GetComponent<UIButtonComponent>().IsPointerInside = true;
                ScriptEngine::OnPointerEvent(hitEntity, ScriptEngine::ScriptPointerEventType::Enter);
            }
        }
        else if (hitEntity && hitEntity.HasComponent<UIButtonComponent>())
        {
            hitEntity.GetComponent<UIButtonComponent>().IsPointerInside = true;
        }

        // Pressed visual: keep pressed on original target even if pointer leaves.
        if (m_UserInterfacePressedEntityIdentifier != 0)
        {
            Entity pressedEntity = GetEntityByUUID(m_UserInterfacePressedEntityIdentifier);
            if (pressedEntity && pressedEntity.HasComponent<UIButtonComponent>())
                pressedEntity.GetComponent<UIButtonComponent>().IsPressed = true;
        }

        if (m_UserInterfacePointerInput.PrimaryButtonPressedThisFrame && hitEntity
            && hitEntity.HasComponent<UIButtonComponent>())
        {
            m_UserInterfacePressedEntityIdentifier = hitIdentifier;
            hitEntity.GetComponent<UIButtonComponent>().IsPressed = true;
            ScriptEngine::OnPointerEvent(hitEntity, ScriptEngine::ScriptPointerEventType::Down);
        }

        if (m_UserInterfacePointerInput.PrimaryButtonReleasedThisFrame)
        {
            Entity pressedEntity = GetEntityByUUID(m_UserInterfacePressedEntityIdentifier);
            if (pressedEntity && pressedEntity.HasComponent<UIButtonComponent>())
            {
                auto& pressedButton = pressedEntity.GetComponent<UIButtonComponent>();
                pressedButton.IsPressed = false;
                ScriptEngine::OnPointerEvent(pressedEntity, ScriptEngine::ScriptPointerEventType::Up);
                if (hitIdentifier == m_UserInterfacePressedEntityIdentifier)
                {
                    pressedButton.WasClickedThisFrame = true;
                    ScriptEngine::OnPointerEvent(pressedEntity, ScriptEngine::ScriptPointerEventType::Click);
                }
            }
            m_UserInterfacePressedEntityIdentifier = 0;
        }
    }

    void Scene::DestroyEntity(entt::entity entityHandle)
    {
        if (!m_Registry.valid(entityHandle))
            return;

        Entity entity{entityHandle, this};
        const std::vector<UUID> childIdentifiers = GetEntityChildren(entity);
        for (UUID childIdentifier : childIdentifiers)
        {
            Entity childEntity = GetEntityByUUID(childIdentifier);
            if (childEntity)
                DestroyEntity((entt::entity)childEntity);
        }

        if (auto *identifierComponent = m_Registry.try_get<IDComponent>(entityHandle))
        {
            auto identifierIterator = m_EntityMap.find(identifierComponent->ID);
            if (identifierIterator != m_EntityMap.end())
                m_EntityMap.erase(identifierIterator);
        }
        // 脚本析构
        if (NativeScriptComponent *nativeScriptComponent = m_Registry.try_get<NativeScriptComponent>(entityHandle))
        {
            if (nativeScriptComponent->Instance)
            {
                nativeScriptComponent->Instance->OnDestroy();
            }
            if (nativeScriptComponent->DestroyScript)
            {
                nativeScriptComponent->DestroyScript(nativeScriptComponent);
            }
        }
        m_Registry.destroy(entityHandle);
        RebuildHierarchyCache();
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

    void Scene::OnUpdateEditor(Timestep ts, EditorCamera &camera, bool drawUserInterfaceContent)
    {
        UpdateSpriteAnimations(ts, true);
        RenderEditorView(camera, drawUserInterfaceContent);
    }

    void Scene::RenderEditorView(EditorCamera& camera, bool drawUserInterfaceContent)
    {
        RenderScene(camera);
        RenderUIInEditor(camera, drawUserInterfaceContent);
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

    void Scene::RunScriptFixedUpdate(Timestep ts)
    {
        auto view = m_Registry.view<ScriptComponent>();
        for (auto entityHandle : view)
        {
            Entity entity = {entityHandle, this};
            ScriptEngine::OnFixedUpdateScript(entity, ts);
        }
    }

    void Scene::OnUpdateRuntime(Timestep ts, bool drawUserInterfaceContent)
    {
        ProcessUserInterfacePointer();

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
            for (auto entityHandle : view)
            {
                Entity entity = {entityHandle, this};
                auto &rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();

                if (rigidbody2D.RuntimeBody)
                {
                    b2BodyId bodyId = ToBodyId(rigidbody2D.RuntimeBody);
                    if (b2Body_IsValid(bodyId))
                    {
                        b2Vec2 position = b2Body_GetPosition(bodyId);
                        b2Rot rotation = b2Body_GetRotation(bodyId);
                        SetEntityWorldTransformFromPhysics(
                                entity, {position.x, position.y}, b2Rot_GetAngle(rotation));
                    }
                }
            }
        }

        RunScriptFixedUpdate(ts);

        // 粒子发射器：按资源与 Transform 发射，再统一更新粒子系统
        if (Project::GetActive())
        {
            auto assetManager = Project::GetAssetManager();
            if (assetManager)
            {
                auto view = m_Registry.group<TransformComponent, ParticleEmitterComponent>();
                for (auto entityHandle : view)
                {
                    Entity particleEntity = {entityHandle, this};
                    auto& emitter = particleEntity.GetComponent<ParticleEmitterComponent>();
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
                    const glm::vec3 worldTranslation = GetEntityWorldTranslation(particleEntity);
                    props.position = worldTranslation;

                    for (int index = 0; index < emitCount; ++index)
                        m_ParticleSystem.Emit(props);
                }
            }
        }
        m_ParticleSystem.OnUpdate(ts);

        RenderGameView(
                m_ViewportWidth, m_ViewportHeight,
                drawUserInterfaceContent);
    }

    bool Scene::RenderGameView(
            uint32_t targetWidth, uint32_t targetHeight,
            bool drawUserInterfaceContent)
    {
        if (targetWidth == 0 || targetHeight == 0)
            return false;

        OnViewportResize(targetWidth, targetHeight);
        Entity cameraEntity = GetPrimaryCameraEntity();
        if (!cameraEntity || !cameraEntity.HasComponent<TransformComponent>())
            return false;

        auto& cameraComponent = cameraEntity.GetComponent<CameraComponent>();
        const glm::mat4 cameraTransform = GetEntityWorldTransformMatrix(cameraEntity);

        RenderCommand::SetDepthTest(true);
        Renderer3D::BeginScene(cameraComponent.Camera, cameraTransform);
        const bool isTwoDimensional =
                Project::GetActive() && Project::GetActive()->GetConfig().Is2D;
        if (m_SkyboxTexture && !isTwoDimensional)
            Renderer3D::DrawSkybox(
                    m_SkyboxTexture, cameraComponent.Camera, cameraTransform);

        auto meshView = m_Registry.view<TransformComponent, MeshComponent>();
        meshView.each(
                [&](entt::entity entityHandle, TransformComponent&, MeshComponent& mesh)
                {
                    const glm::mat4 worldTransform =
                            GetEntityWorldTransformMatrix({entityHandle, this});
                    if (mesh.Type == MeshComponent::MeshType::Cube)
                        Renderer3D::DrawCube(worldTransform, mesh.Color, (int)entityHandle);
                    else if (mesh.Type == MeshComponent::MeshType::Plane)
                        Renderer3D::DrawPlane(worldTransform, mesh.Color, (int)entityHandle);
                    else if (mesh.Type == MeshComponent::MeshType::Sphere)
                        Renderer3D::DrawSphere(worldTransform, mesh.Color, (int)entityHandle);
                    else if (mesh.Type == MeshComponent::MeshType::Capsule)
                        Renderer3D::DrawCapsule(worldTransform, mesh.Color, (int)entityHandle);
                });
        Renderer3D::EndScene();

        RenderCommand::SetDepthTest(true);
        Renderer2D::BeginScene(cameraComponent.Camera, cameraTransform);
        {
            auto assetManager = Project::TryGetAssetManager();
            DrawSpriteRenderersSorted(this, m_Registry, assetManager.get());
        }

        if (Project::GetActive())
        {
            auto assetManager = Project::GetAssetManager();
            auto tilemapView = m_Registry.view<TransformComponent, TilemapComponent>();
            tilemapView.each(
                    [&](entt::entity entityHandle, TransformComponent&, TilemapComponent& tilemap)
                    {
                        if (!assetManager || tilemap.TileMapHandle == 0)
                            return;
                        Ref<Asset> mapAsset = assetManager->GetAsset(tilemap.TileMapHandle);
                        if (!mapAsset)
                            return;
                        Ref<TileMapData> mapData = std::static_pointer_cast<TileMapData>(mapAsset);
                        Ref<TileSet> tileSet;
                        if (mapData->GetTileSetHandle() != 0)
                        {
                            Ref<Asset> tileSetAsset =
                                    assetManager->GetAsset(mapData->GetTileSetHandle());
                            if (tileSetAsset)
                                tileSet = std::static_pointer_cast<TileSet>(tileSetAsset);
                        }
                        Renderer2D::DrawTilemap(
                                GetEntityWorldTransformMatrix({entityHandle, this}),
                                mapData, tileSet, (int)entityHandle);
                    });
        }

        auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();
        circleView.each(
                [&](entt::entity entityHandle, TransformComponent&, CircleRendererComponent& circle)
                {
                    Renderer2D::DrawCircle(
                            GetEntityWorldTransformMatrix({entityHandle, this}),
                            circle.Color, circle.Thickness, circle.Fade, (int)entityHandle);
                });

        auto particleAssetManager = Project::GetActive() ? Project::GetAssetManager() : nullptr;
        m_ParticleSystem.ForEachAlive(
                [&](const ParticleSystem::ParticleView& particle)
                {
                    const float lifetimeProgress =
                            1.0f - particle.remainingLife / particle.lifetime;
                    const glm::vec4 color =
                            glm::mix(particle.colorBegin, particle.colorEnd, lifetimeProgress);
                    const float size =
                            glm::mix(particle.sizeBegin, particle.sizeEnd, lifetimeProgress);
                    const glm::mat4 transform =
                            glm::translate(glm::mat4(1.0f), particle.position)
                            * glm::rotate(
                                    glm::mat4(1.0f), particle.rotation,
                                    glm::vec3(0.0f, 0.0f, 1.0f))
                            * glm::scale(glm::mat4(1.0f), glm::vec3(size));

                    Ref<Texture2D> texture;
                    if (particle.textureHandle != 0 && particleAssetManager
                        && particleAssetManager->IsAssetHandleValid(
                                static_cast<AssetHandle>(particle.textureHandle)))
                    {
                        texture = std::dynamic_pointer_cast<Texture2D>(
                                particleAssetManager->GetAsset(
                                        static_cast<AssetHandle>(particle.textureHandle)));
                    }

                    if (particle.shape == ParticleShape::Circle)
                        Renderer2D::DrawCircle(transform, color, 1.0f, 0.0025f);
                    else if (texture)
                        Renderer2D::DrawQuad(transform, texture, 1.0f, color);
                    else
                        Renderer2D::DrawQuad(transform, color);
                });
        Renderer2D::EndScene();

        if (drawUserInterfaceContent)
            RenderUI(static_cast<float>(targetWidth), static_cast<float>(targetHeight));
        return true;
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
            for (auto entityHandle : view)
            {
                Entity entity = {entityHandle, this};
                auto &rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();

                if (rigidbody2D.RuntimeBody)
                {
                    b2BodyId bodyId = ToBodyId(rigidbody2D.RuntimeBody);
                    if (b2Body_IsValid(bodyId))
                    {
                        b2Vec2 position = b2Body_GetPosition(bodyId);
                        b2Rot rotation = b2Body_GetRotation(bodyId);
                        SetEntityWorldTransformFromPhysics(
                                entity, {position.x, position.y}, b2Rot_GetAngle(rotation));
                    }
                }
            }
        }

        UpdateSpriteAnimations(ts, false);
        RunScriptFixedUpdate(ts);
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
            if constexpr (std::is_same_v<Component, TransformComponent>)
            {
                if (dst.all_of<RectTransformComponent>(dstEnttID))
                    continue;
            }
            else if constexpr (std::is_same_v<Component, RectTransformComponent>)
            {
                if (dst.all_of<TransformComponent>(dstEnttID))
                    continue;
            }
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

        // Create entities in new scene（UI / 世界分域创建，避免 UI 被挂上默认 Transform）
        auto idView = srcSceneRegistry.view<IDComponent>();
        for (auto e: idView)
        {
            UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
            const auto &name = srcSceneRegistry.get<TagComponent>(e).Tag;
            const bool isUserInterfaceEntity = srcSceneRegistry.all_of<RectTransformComponent>(e);
            Entity newEntity = isUserInterfaceEntity ? newScene->CreateUIEntityWithUUID(uuid, name)
                                                     : newScene->CreateEntityWithUUID(uuid, name);
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
        CopyComponent<RelationshipComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<RectTransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CanvasComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<UIImageComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<UITextComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<UIButtonComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

        newScene->RebuildHierarchyCache();

        return newScene;
    }

    Entity Scene::DuplicateEntity(Entity entity)
    {
        std::string name = entity.GetName();
        const bool isUserInterfaceEntity = entity.HasComponent<RectTransformComponent>();
        Entity newEntity = isUserInterfaceEntity ? CreateUIEntity(name) : CreateEntity(name);

        if (!isUserInterfaceEntity && entity.HasComponent<TransformComponent>())
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

        // UI：CreateUIEntity 已带 UITransform，只需覆盖数值
        if (isUserInterfaceEntity && entity.HasComponent<RectTransformComponent>())
            newEntity.GetComponent<RectTransformComponent>() = entity.GetComponent<RectTransformComponent>();
        if (entity.HasComponent<CanvasComponent>())
            newEntity.AddComponent<CanvasComponent>(entity.GetComponent<CanvasComponent>());
        if (entity.HasComponent<UIImageComponent>())
            newEntity.AddComponent<UIImageComponent>(entity.GetComponent<UIImageComponent>());
        if (entity.HasComponent<UITextComponent>())
            newEntity.AddComponent<UITextComponent>(entity.GetComponent<UITextComponent>());
        if (entity.HasComponent<UIButtonComponent>())
        {
            UIButtonComponent copiedButton = entity.GetComponent<UIButtonComponent>();
            copiedButton.IsPointerInside = false;
            copiedButton.IsPressed = false;
            copiedButton.WasClickedThisFrame = false;
            newEntity.AddComponent<UIButtonComponent>(copiedButton);
        }

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

    Entity Scene::GetEntityByUUID(UUID uuid) const
    {
        return const_cast<Scene*>(this)->GetEntityByUUID(uuid);
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
        auto view = m_Registry.view<TransformComponent, CameraComponent>();
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

    void Scene::SyncEntityTransformToPhysics(Entity entity)
    {
        if (!b2World_IsValid(m_Box2DWorld))
            return;

        if (!entity || !entity.HasComponent<Rigidbody2DComponent>())
            return;

        auto& rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();
        if (!rigidbody2D.RuntimeBody)
            return;

        b2BodyId bodyId = ToBodyId(rigidbody2D.RuntimeBody);
        if (!b2Body_IsValid(bodyId))
            return;

        const glm::vec3 worldTranslation = GetEntityWorldTranslation(entity);
        const glm::vec3 worldRotation = GetEntityWorldRotation(entity);
        b2Body_SetTransform(
                bodyId,
                {worldTranslation.x, worldTranslation.y},
                b2MakeRot(worldRotation.z));
    }

    void Scene::OnPhysics2DStart()
    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2{0.0f, -9.8f};
        m_Box2DWorld = b2CreateWorld(&worldDef);

        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto entityHandle : view)
        {
            Entity entity = {entityHandle, this};
            auto &rigidbody2D = entity.GetComponent<Rigidbody2DComponent>();
            const glm::vec3 worldTranslation = GetEntityWorldTranslation(entity);
            const glm::vec3 worldRotation = GetEntityWorldRotation(entity);
            const glm::vec3 worldScale = GetEntityWorldScale(entity);

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

            bodyDef.position = {worldTranslation.x, worldTranslation.y};
            bodyDef.rotation = b2MakeRot(worldRotation.z);
            bodyDef.fixedRotation = rigidbody2D.FixedRotation;
            bodyDef.userData = (void *)(uintptr_t)(uint32_t)entity;

            b2BodyId bodyId = b2CreateBody(m_Box2DWorld, &bodyDef);
            rigidbody2D.RuntimeBody = ToPtr(bodyId);

            if (entity.HasComponent<BoxCollider2DComponent>())
                AttachBoxColliderToBody(bodyId, entity.GetComponent<BoxCollider2DComponent>(), worldScale);

            if (entity.HasComponent<CircleCollider2DComponent>())
                AttachCircleColliderToBody(bodyId, entity.GetComponent<CircleCollider2DComponent>(), worldScale);
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
                const glm::vec3 worldScale = GetEntityWorldScale(entity);
                const b2BodyId staticBodyId = CreateStaticPhysicsBody(m_Box2DWorld, this, entity);

                if (hasBoxCollider)
                    AttachBoxColliderToBody(
                            staticBodyId, entity.GetComponent<BoxCollider2DComponent>(), worldScale);
                if (hasCircleCollider)
                    AttachCircleColliderToBody(
                            staticBodyId, entity.GetComponent<CircleCollider2DComponent>(), worldScale);
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
                const bool isTriggerContact =
                        b2Shape_IsSensor(ev.shapeIdA) || b2Shape_IsSensor(ev.shapeIdB);

                auto buildCollisionInterop = [&](Entity self, Entity other) -> Collision2DInterop
                {
                    Collision2DInterop collisionInterop;
                    collisionInterop.OtherEntityID = other.GetUUID();

                    const Entity shapeOwnerA = GetEntityFromShape(ev.shapeIdA);
                    const b2Vec2 &manifoldNormal = ev.manifold.normal;
                    if (shapeOwnerA == self)
                        collisionInterop.Normal = glm::vec2{-manifoldNormal.x, -manifoldNormal.y};
                    else
                        collisionInterop.Normal = glm::vec2{manifoldNormal.x, manifoldNormal.y};

                    collisionInterop.Point = glm::vec2{0.0f, 0.0f};
                    if (ev.manifold.pointCount > 0)
                    {
                        const b2Vec2 &contactPoint = ev.manifold.points[0].point;
                        collisionInterop.Point = glm::vec2{contactPoint.x, contactPoint.y};
                    }

                    return collisionInterop;
                };

                if (isTriggerContact)
                {
                    ScriptEngine::OnTriggerEnter2D(entityA, entityB, buildCollisionInterop(entityA, entityB));
                    ScriptEngine::OnTriggerEnter2D(entityB, entityA, buildCollisionInterop(entityB, entityA));
                }
                else
                {
                    ScriptEngine::OnCollisionEnter2D(entityA, entityB, buildCollisionInterop(entityA, entityB));
                    ScriptEngine::OnCollisionEnter2D(entityB, entityA, buildCollisionInterop(entityB, entityA));
                }
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
                const bool isTriggerContact =
                        b2Shape_IsSensor(ev.shapeIdA) || b2Shape_IsSensor(ev.shapeIdB);

                if (isTriggerContact)
                {
                    ScriptEngine::OnTriggerExit2D(entityA, entityB);
                    ScriptEngine::OnTriggerExit2D(entityB, entityA);
                }
                else
                {
                    ScriptEngine::OnCollisionExit2D(entityA, entityB);
                    ScriptEngine::OnCollisionExit2D(entityB, entityA);
                }
            }
        }
    }

    void Scene::RenderScene(EditorCamera &camera)
    {
        Renderer2D::BeginScene(camera);

        // Draw Sprites
        {
            auto assetManager = Project::TryGetAssetManager();
            DrawSpriteRenderersSorted(this, m_Registry, assetManager.get());
        }
        
        // Draw Tilemaps
        {
            if (Project::GetActive())
            {
                auto assetManager = Project::GetAssetManager();
                auto view = m_Registry.view<TransformComponent, TilemapComponent>();
                view.each([&](entt::entity entityHandle, TransformComponent&, TilemapComponent &tilemap)
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

                    Himii::Renderer2D::DrawTilemap(GetEntityWorldTransformMatrix({entityHandle, this}), mapData,
                                                     tileSet, (int)entityHandle);
                });
            }
        }

        // Draw Circles
        {
            auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
            view.each(
                    [&](entt::entity entityHandle, TransformComponent&, CircleRendererComponent &circle) {
                        Himii::Renderer2D::DrawCircle(GetEntityWorldTransformMatrix({entityHandle, this}),
                                                      circle.Color, circle.Thickness, circle.Fade, (int)entityHandle);
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
            view.each([&](entt::entity entityHandle, TransformComponent&, MeshComponent &mesh)
            {
                const glm::mat4 worldTransform = GetEntityWorldTransformMatrix({entityHandle, this});
                if (mesh.Type == MeshComponent::MeshType::Cube)
                    Renderer3D::DrawCube(worldTransform, mesh.Color, (int)entityHandle);
                else if (mesh.Type == MeshComponent::MeshType::Plane)
                    Renderer3D::DrawPlane(worldTransform, mesh.Color, (int)entityHandle);
                else if (mesh.Type == MeshComponent::MeshType::Sphere)
                    Renderer3D::DrawSphere(worldTransform, mesh.Color, (int)entityHandle);
                else if (mesh.Type == MeshComponent::MeshType::Capsule)
                    Renderer3D::DrawCapsule(worldTransform, mesh.Color, (int)entityHandle);
            });
            Renderer3D::EndScene();
        }
    }

    void Scene::RenderUI(float viewportWidth, float viewportHeight)
    {
        if (viewportWidth == 0.0f || viewportHeight == 0.0f)
            return;

        Entity canvasEntity = FindCanvasEntity();
        if (!canvasEntity)
            return;

        const glm::mat4 projection = glm::ortho(0.0f, viewportWidth, 0.0f, viewportHeight, -1.0f, 1.0f);
        const glm::mat4 view = glm::mat4(1.0f);
        const glm::mat4 canvasToScreen = GetCanvasToScreenMatrix(viewportWidth, viewportHeight);

        PrepareUserInterfaceFonts();
        RenderCommand::SetDepthTest(false);
        Renderer2D::BeginScene(projection, view);
        RenderUIElements(canvasToScreen, viewportWidth, viewportHeight);
        Renderer2D::EndScene();
        RenderCommand::SetDepthTest(true);
    }

    void Scene::PrepareUserInterfaceFonts()
    {
        struct FontTextCollection
        {
            Ref<Font> FontAsset;
            std::string CombinedText;
        };

        std::unordered_map<Font*, FontTextCollection> textByFont;
        auto textView = m_Registry.view<RectTransformComponent, UITextComponent>();
        for (entt::entity entityHandle : textView)
        {
            Entity entity{entityHandle, this};
            if (!IsEntityUnderCanvas(entity))
                continue;
            const auto& text = entity.GetComponent<UITextComponent>();
            Ref<Font> fontAsset = text.FontAsset ? text.FontAsset : Font::GetDefault();
            if (!fontAsset)
                continue;

            FontTextCollection& collection = textByFont[fontAsset.get()];
            collection.FontAsset = fontAsset;
            collection.CombinedText += text.TextString;
        }

        for (auto& [fontPointer, collection] : textByFont)
        {
            (void)fontPointer;
            collection.FontAsset->EnsureGlyphsForText(collection.CombinedText);
        }
    }

    void Scene::RenderUIElements(
            const glm::mat4& designToTargetMatrix, float targetWidth, float targetHeight)
    {
        Entity canvasEntity = FindCanvasEntity();
        if (!canvasEntity)
            return;

        enum class UserInterfacePrimitiveType
        {
            None = 0,
            Image,
            Text
        };
        UserInterfacePrimitiveType lastPrimitiveType = UserInterfacePrimitiveType::None;
        const auto flushIfPrimitiveTypeChanged =
                [&](UserInterfacePrimitiveType nextPrimitiveType)
        {
            if (lastPrimitiveType != UserInterfacePrimitiveType::None
                && lastPrimitiveType != nextPrimitiveType)
            {
                // 有序命令流：相邻不同类型批次强制分隔，保持 SiblingIndex 顺序。
                Renderer2D::FlushCurrentBatch();
            }
            lastPrimitiveType = nextPrimitiveType;
        };

        std::function<void(Entity)> drawUserInterfaceSubtree;
        drawUserInterfaceSubtree = [&](Entity entity)
        {
            if (!entity || !entity.HasComponent<RectTransformComponent>())
                return;

            const ResolvedRectTransform resolvedRectTransform =
                    ResolveRectTransform(entity, targetWidth, targetHeight);
            if (!resolvedRectTransform.Valid)
                return;

            if (entity.HasComponent<UIImageComponent>())
            {
                flushIfPrimitiveTypeChanged(UserInterfacePrimitiveType::Image);
                auto& image = entity.GetComponent<UIImageComponent>();
                const glm::mat4 drawTransform = designToTargetMatrix
                        * resolvedRectTransform.WorldTransform
                        * glm::scale(glm::mat4(1.0f),
                                     glm::vec3(resolvedRectTransform.Size, 1.0f));

                glm::vec4 drawColor = image.Color;
                if (entity.HasComponent<UIButtonComponent>())
                    drawColor = entity.GetComponent<UIButtonComponent>().EvaluateEffectiveColor(
                            image.Color);

                if (image.Texture)
                    Renderer2D::DrawQuad(
                            drawTransform, image.Texture, 1.0f, drawColor, (int)(entt::entity)entity);
                else
                    Renderer2D::DrawQuad(drawTransform, drawColor, (int)(entt::entity)entity);
            }

            if (entity.HasComponent<UITextComponent>())
            {
                flushIfPrimitiveTypeChanged(UserInterfacePrimitiveType::Text);
                auto& text = entity.GetComponent<UITextComponent>();
                Ref<Font> fontAsset = text.FontAsset ? text.FontAsset : Font::GetDefault();
                if (fontAsset)
                {
                    fontAsset->ProcessCompletedGenerations();
                    const glm::mat4 drawTransform = designToTargetMatrix
                            * resolvedRectTransform.WorldTransform;
                    TextLayoutSettings layoutSettings;
                    layoutSettings.RectangleSize = resolvedRectTransform.Size;
                    layoutSettings.FontSize = std::max(text.FontSize, 1.0f);
                    layoutSettings.Kerning = text.Kerning;
                    layoutSettings.LineSpacing = text.LineSpacing;
                    layoutSettings.HorizontalAlignment = text.HorizontalAlignment;
                    layoutSettings.VerticalAlignment = text.VerticalAlignment;
                    Renderer2D::DrawStringInRectangle(
                            text.TextString, fontAsset, drawTransform,
                            layoutSettings, text.Color,
                            (int)(entt::entity)entity);
                }
            }

            for (UUID childIdentifier : GetEntityChildren(entity))
                drawUserInterfaceSubtree(GetEntityByUUID(childIdentifier));
        };

        for (UUID childIdentifier : GetEntityChildren(canvasEntity))
            drawUserInterfaceSubtree(GetEntityByUUID(childIdentifier));
    }

    Scene::OrthographicViewBounds Scene::GetPrimaryOrthographicViewBounds() const
    {
        OrthographicViewBounds bounds;
        Entity cameraEntity = const_cast<Scene*>(this)->GetPrimaryCameraEntity();
        if (!cameraEntity || !cameraEntity.HasComponent<CameraComponent>()
            || !cameraEntity.HasComponent<TransformComponent>())
            return bounds;

        auto& cameraComponent = cameraEntity.GetComponent<CameraComponent>();
        if (cameraComponent.Camera.GetProjectionType() != SceneCamera::ProjectionType::Orthographic)
            return bounds;

        const float orthographicSize = cameraComponent.Camera.GetOrthographicSize();
        const float aspectRatio = (m_ViewportHeight > 0)
                                         ? static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight)
                                         : 1.0f;

        bounds.Valid = true;
        bounds.CameraWorldMatrix = GetEntityWorldTransformMatrix(cameraEntity);
        bounds.Left = -orthographicSize * aspectRatio * 0.5f;
        bounds.Right = orthographicSize * aspectRatio * 0.5f;
        bounds.Bottom = -orthographicSize * 0.5f;
        bounds.Top = orthographicSize * 0.5f;
        return bounds;
    }

    Scene::DesignFrameWorldBounds Scene::GetDesignFrameWorldBounds() const
    {
        DesignFrameWorldBounds designBounds;
        Entity canvasEntity = FindCanvasEntity();
        if (!canvasEntity || !canvasEntity.HasComponent<CanvasComponent>())
            return designBounds;

        const float targetWidth = std::max(static_cast<float>(m_ViewportWidth), 1.0f);
        const float targetHeight = std::max(static_cast<float>(m_ViewportHeight), 1.0f);
        const CanvasLayoutContext layoutContext =
                GetCanvasLayoutContext(targetWidth, targetHeight);
        if (!layoutContext.Valid)
            return designBounds;

        float cameraWidth = targetWidth * 0.01f;
        float cameraHeight = targetHeight * 0.01f;
        OrthographicViewBounds cameraBounds = GetPrimaryOrthographicViewBounds();
        if (cameraBounds.Valid)
        {
            cameraWidth = cameraBounds.Right - cameraBounds.Left;
            cameraHeight = cameraBounds.Top - cameraBounds.Bottom;
        }

        const float targetAspectRatio = targetWidth / targetHeight;
        float frameWidth = cameraWidth;
        float frameHeight = frameWidth / targetAspectRatio;
        if (frameHeight > cameraHeight)
        {
            frameHeight = cameraHeight;
            frameWidth = frameHeight * targetAspectRatio;
        }
        const float designToWorldScale =
                frameWidth / std::max(layoutContext.LogicalSize.x, 1.0e-5f);

        designBounds.Valid = true;
        designBounds.DesignToWorldScale = designToWorldScale;
        designBounds.HalfWidth = frameWidth * 0.5f;
        designBounds.HalfHeight = frameHeight * 0.5f;
        return designBounds;
    }

    glm::mat4 Scene::GetDesignToWorldMatrix() const
    {
        DesignFrameWorldBounds designBounds = GetDesignFrameWorldBounds();
        if (!designBounds.Valid)
            return glm::mat4(1.0f);

        // 设计中心原点 (0,0) → 世界原点；XY 平面 z=0。
        return glm::scale(glm::mat4(1.0f),
                          glm::vec3(designBounds.DesignToWorldScale, designBounds.DesignToWorldScale, 1.0f));
    }

    void Scene::DrawPrimaryCameraBounds(const glm::vec4& color) const
    {
        OrthographicViewBounds bounds = GetPrimaryOrthographicViewBounds();
        if (!bounds.Valid)
            return;

        // 画在 z=0 玩法平面，避免主相机在 z=10 时 Edit 拉近后框落到相机背后消失。
        const glm::vec3 cameraTranslation = glm::vec3(bounds.CameraWorldMatrix[3]);
        const glm::vec3 bottomLeft{cameraTranslation.x + bounds.Left, cameraTranslation.y + bounds.Bottom, 0.0f};
        const glm::vec3 bottomRight{cameraTranslation.x + bounds.Right, cameraTranslation.y + bounds.Bottom, 0.0f};
        const glm::vec3 topRight{cameraTranslation.x + bounds.Right, cameraTranslation.y + bounds.Top, 0.0f};
        const glm::vec3 topLeft{cameraTranslation.x + bounds.Left, cameraTranslation.y + bounds.Top, 0.0f};

        Renderer2D::DrawLine(bottomLeft, bottomRight, color);
        Renderer2D::DrawLine(bottomRight, topRight, color);
        Renderer2D::DrawLine(topRight, topLeft, color);
        Renderer2D::DrawLine(topLeft, bottomLeft, color);
    }

    void Scene::DrawCanvasDesignBounds(const glm::vec4& color) const
    {
        DesignFrameWorldBounds designBounds = GetDesignFrameWorldBounds();
        if (!designBounds.Valid)
            return;

        const glm::vec3 bottomLeft{-designBounds.HalfWidth, -designBounds.HalfHeight, 0.0f};
        const glm::vec3 bottomRight{designBounds.HalfWidth, -designBounds.HalfHeight, 0.0f};
        const glm::vec3 topRight{designBounds.HalfWidth, designBounds.HalfHeight, 0.0f};
        const glm::vec3 topLeft{-designBounds.HalfWidth, designBounds.HalfHeight, 0.0f};

        Renderer2D::DrawLine(bottomLeft, bottomRight, color);
        Renderer2D::DrawLine(bottomRight, topRight, color);
        Renderer2D::DrawLine(topRight, topLeft, color);
        Renderer2D::DrawLine(topLeft, bottomLeft, color);
    }

    void Scene::RenderUIInEditor(EditorCamera& editorCamera, bool drawUserInterfaceContent)
    {
        if (drawUserInterfaceContent)
            PrepareUserInterfaceFonts();
        RenderCommand::SetDepthTest(false);
        Renderer2D::BeginScene(editorCamera);

        DrawPrimaryCameraBounds(glm::vec4(0.95f, 0.85f, 0.2f, 0.9f));
        DrawCanvasDesignBounds(glm::vec4(0.35f, 0.85f, 0.95f, 0.9f));

        if (drawUserInterfaceContent && FindCanvasEntity())
            RenderUIElements(
                    GetDesignToWorldMatrix(),
                    static_cast<float>(m_ViewportWidth),
                    static_cast<float>(m_ViewportHeight));

        Renderer2D::EndScene();
        RenderCommand::SetDepthTest(true);
    }

    bool Scene::IsWorldPositionInsideDesignFrame(const glm::vec2& worldPosition) const
    {
        DesignFrameWorldBounds designBounds = GetDesignFrameWorldBounds();
        if (!designBounds.Valid)
            return false;

        return worldPosition.x >= -designBounds.HalfWidth && worldPosition.x <= designBounds.HalfWidth
               && worldPosition.y >= -designBounds.HalfHeight && worldPosition.y <= designBounds.HalfHeight;
    }

    Entity Scene::FindCanvasEntity() const
    {
        auto canvasView = m_Registry.view<CanvasComponent>();
        for (auto entityHandle : canvasView)
            return Entity{entityHandle, const_cast<Scene*>(this)};
        return {};
    }

    bool Scene::IsEntityUnderCanvas(Entity entity) const
    {
        Entity canvasEntity = FindCanvasEntity();
        if (!entity || !canvasEntity)
            return false;
        if (entity == canvasEntity)
            return true;
        return IsEntityDescendantOf(entity, canvasEntity);
    }

    Scene::CanvasLayoutContext Scene::GetCanvasLayoutContext(
            float targetWidth, float targetHeight) const
    {
        CanvasLayoutContext context;
        Entity canvasEntity = FindCanvasEntity();
        if (!canvasEntity || targetWidth <= 0.0f || targetHeight <= 0.0f)
            return context;

        const auto& canvas = canvasEntity.GetComponent<CanvasComponent>();
        context.Valid = true;
        context.TargetSize = {targetWidth, targetHeight};
        context.ScaleFactor = canvas.ScaleMode == CanvasScaleMode::ConstantPixelSize
                                      ? 1.0f
                                      : ComputeCanvasScaleFactor(targetWidth, targetHeight);
        context.LogicalSize = context.TargetSize / std::max(context.ScaleFactor, 1.0e-5f);
        return context;
    }

    Scene::ResolvedRectTransform Scene::ResolveRectTransform(
            Entity entity, float targetWidth, float targetHeight) const
    {
        ResolvedRectTransform result;
        if (!entity || !entity.HasComponent<RectTransformComponent>() || !IsEntityUnderCanvas(entity))
            return result;

        const CanvasLayoutContext context = GetCanvasLayoutContext(targetWidth, targetHeight);
        if (!context.Valid)
            return result;

        Entity canvasEntity = FindCanvasEntity();
        if (entity == canvasEntity)
        {
            result.Valid = true;
            result.Size = context.LogicalSize;
            result.WorldTransform = glm::mat4(1.0f);
            auto& rectTransform = entity.GetComponent<RectTransformComponent>();
            rectTransform.ResolvedSize = result.Size;
            rectTransform.CachedWorldTransform = result.WorldTransform;
            rectTransform.WorldTransformDirty = false;
            return result;
        }

        Entity parentEntity = GetParentEntity(entity);
        if (!parentEntity || !parentEntity.HasComponent<RectTransformComponent>())
            return result;

        const ResolvedRectTransform parentResult =
                ResolveRectTransform(parentEntity, targetWidth, targetHeight);
        if (!parentResult.Valid)
            return result;

        auto& rectTransform = entity.GetComponent<RectTransformComponent>();
        const glm::vec2 anchorMinimum = glm::clamp(
                rectTransform.AnchorMinimum, glm::vec2(0.0f), glm::vec2(1.0f));
        const glm::vec2 anchorMaximum = glm::clamp(
                rectTransform.AnchorMaximum, anchorMinimum, glm::vec2(1.0f));
        const glm::vec2 pivot = glm::clamp(rectTransform.Pivot, glm::vec2(0.0f), glm::vec2(1.0f));

        const glm::vec2 anchorRange = anchorMaximum - anchorMinimum;
        const glm::vec2 resolvedSize =
                glm::max(parentResult.Size * anchorRange + rectTransform.SizeDelta, glm::vec2(0.01f));
        const glm::vec2 anchorReference =
                -parentResult.Size * 0.5f
                + parentResult.Size
                          * (anchorMinimum + anchorRange * pivot);
        const glm::vec2 pivotPosition = anchorReference + rectTransform.AnchoredPosition;
        const glm::vec2 rectCenter = pivotPosition + (glm::vec2(0.5f) - pivot) * resolvedSize;

        const glm::mat4 localTransform =
                glm::translate(glm::mat4(1.0f), glm::vec3(rectCenter, 0.0f))
                * glm::rotate(
                        glm::mat4(1.0f), rectTransform.RotationRadians,
                        glm::vec3(0.0f, 0.0f, 1.0f));

        result.Valid = true;
        result.Size = resolvedSize;
        result.WorldTransform = parentResult.WorldTransform * localTransform;
        rectTransform.ResolvedSize = resolvedSize;
        rectTransform.CachedWorldTransform = result.WorldTransform;
        rectTransform.WorldTransformDirty = false;
        return result;
    }

    float Scene::ComputeCanvasScaleFactor(float viewportWidth, float viewportHeight) const
    {
        Entity canvasEntity = FindCanvasEntity();
        if (!canvasEntity)
            return 1.0f;

        const auto& canvas = canvasEntity.GetComponent<CanvasComponent>();
        const float referenceWidth = std::max(canvas.ReferenceResolution.x, 1.0f);
        const float referenceHeight = std::max(canvas.ReferenceResolution.y, 1.0f);
        const float scaleWidth = viewportWidth / referenceWidth;
        const float scaleHeight = viewportHeight / referenceHeight;
        const float match = glm::clamp(canvas.MatchWidthOrHeight, 0.0f, 1.0f);

        // Unity Scale With Screen Size（对数加权）
        const float logWidth = std::log2(std::max(scaleWidth, 1.0e-5f));
        const float logHeight = std::log2(std::max(scaleHeight, 1.0e-5f));
        const float logWeighted = glm::mix(logWidth, logHeight, match);
        return std::pow(2.0f, logWeighted);
    }

    glm::mat4 Scene::GetCanvasToScreenMatrix(float viewportWidth, float viewportHeight) const
    {
        if (!FindCanvasEntity())
            return glm::mat4(1.0f);

        const CanvasLayoutContext context = GetCanvasLayoutContext(viewportWidth, viewportHeight);
        if (!context.Valid)
            return glm::mat4(1.0f);
        // 设计中心原点 → 屏幕中心；letterbox 由 scaleFactor 保证等比。
        return glm::translate(glm::mat4(1.0f), glm::vec3(viewportWidth * 0.5f, viewportHeight * 0.5f, 0.0f))
               * glm::scale(
                       glm::mat4(1.0f),
                       glm::vec3(context.ScaleFactor, context.ScaleFactor, 1.0f));
    }

    void Scene::SyncCanvasReferenceResolutionToTransform(Entity canvasEntity)
    {
        if (!canvasEntity || !canvasEntity.HasComponent<CanvasComponent>()
            || !canvasEntity.HasComponent<RectTransformComponent>())
            return;

        const auto& canvas = canvasEntity.GetComponent<CanvasComponent>();
        auto& userInterfaceTransform = canvasEntity.GetComponent<RectTransformComponent>();
        // Canvas 根节点锁定在设计空间原点；Size 仅镜像参考分辨率，不参与矩阵。
        userInterfaceTransform.AnchoredPosition = glm::vec2(0.0f);
        userInterfaceTransform.RotationRadians = 0.0f;
        userInterfaceTransform.AnchorMinimum = glm::vec2(0.0f);
        userInterfaceTransform.AnchorMaximum = glm::vec2(1.0f);
        userInterfaceTransform.Pivot = glm::vec2(0.5f);
        userInterfaceTransform.SizeDelta = glm::vec2(0.0f);
        userInterfaceTransform.ResolvedSize = canvas.ReferenceResolution;
        MarkEntityTransformDirty(canvasEntity);
    }

    bool Scene::EntitiesShareTransformDomain(Entity left, Entity right) const
    {
        if (!left || !right)
            return false;

        const bool leftIsUserInterface = left.HasComponent<RectTransformComponent>();
        const bool rightIsUserInterface = right.HasComponent<RectTransformComponent>();
        return leftIsUserInterface == rightIsUserInterface;
    }

    void Scene::RebuildHierarchyCache()
    {
        m_ChildrenCache.clear();

        auto relationshipView = m_Registry.view<RelationshipComponent>();
        for (auto entityHandle : relationshipView)
        {
            Entity entity{entityHandle, this};
            const UUID parentIdentifier = entity.GetComponent<RelationshipComponent>().Parent;
            if (parentIdentifier == 0)
                continue;

            m_ChildrenCache[parentIdentifier].push_back(entity.GetUUID());
        }

        for (auto& [parentIdentifier, children] : m_ChildrenCache)
        {
            (void)parentIdentifier;
            std::sort(
                    children.begin(), children.end(),
                    [&](UUID leftIdentifier, UUID rightIdentifier)
                    {
                        Entity leftEntity = GetEntityByUUID(leftIdentifier);
                        Entity rightEntity = GetEntityByUUID(rightIdentifier);
                        const uint32_t leftSiblingIndex =
                                leftEntity && leftEntity.HasComponent<RelationshipComponent>()
                                        ? leftEntity.GetComponent<RelationshipComponent>().SiblingIndex
                                        : 0;
                        const uint32_t rightSiblingIndex =
                                rightEntity && rightEntity.HasComponent<RelationshipComponent>()
                                        ? rightEntity.GetComponent<RelationshipComponent>().SiblingIndex
                                        : 0;
                        if (leftSiblingIndex != rightSiblingIndex)
                            return leftSiblingIndex < rightSiblingIndex;
                        return static_cast<uint64_t>(leftIdentifier)
                               < static_cast<uint64_t>(rightIdentifier);
                    });
        }
    }

    Entity Scene::GetParentEntity(Entity entity) const
    {
        if (!entity || !entity.HasComponent<RelationshipComponent>())
            return {};

        const UUID parentIdentifier = entity.GetComponent<RelationshipComponent>().Parent;
        if (parentIdentifier == 0)
            return {};

        return const_cast<Scene*>(this)->GetEntityByUUID(parentIdentifier);
    }

    const std::vector<UUID>& Scene::GetEntityChildren(Entity entity) const
    {
        if (!entity)
            return s_EmptyChildrenList;

        const auto iterator = m_ChildrenCache.find(entity.GetUUID());
        if (iterator == m_ChildrenCache.end())
            return s_EmptyChildrenList;

        return iterator->second;
    }

    std::vector<Entity> Scene::GetRootEntities(bool userInterfaceEntities) const
    {
        std::vector<Entity> rootEntities;
        auto tagView = m_Registry.view<TagComponent>();
        for (auto entityHandle : tagView)
        {
            Entity entity{entityHandle, const_cast<Scene*>(this)};
            const bool isUserInterfaceEntity = entity.HasComponent<RectTransformComponent>();
            if (isUserInterfaceEntity != userInterfaceEntities)
                continue;

            if (entity.HasComponent<RelationshipComponent>()
                && entity.GetComponent<RelationshipComponent>().Parent != 0)
                continue;

            rootEntities.push_back(entity);
        }

        return rootEntities;
    }

    bool Scene::IsEntityDescendantOf(Entity potentialDescendant, Entity potentialAncestor) const
    {
        if (!potentialDescendant || !potentialAncestor)
            return false;

        Entity currentEntity = GetParentEntity(potentialDescendant);
        while (currentEntity)
        {
            if (currentEntity == potentialAncestor)
                return true;
            currentEntity = GetParentEntity(currentEntity);
        }

        return false;
    }

    glm::mat4 Scene::GetEntityWorldTransformMatrix(Entity entity) const
    {
        if (!entity)
            return glm::mat4(1.0f);

        if (entity.HasComponent<TransformComponent>())
        {
            auto& transform = entity.GetComponent<TransformComponent>();
            if (transform.WorldTransformDirty)
            {
                Entity parentEntity = GetParentEntity(entity);
                if (parentEntity)
                {
                    transform.CachedWorldTransform =
                            GetEntityWorldTransformMatrix(parentEntity) * transform.GetLocalTransform();
                }
                else
                {
                    transform.CachedWorldTransform = transform.GetLocalTransform();
                }
                transform.WorldTransformDirty = false;
            }

            return transform.CachedWorldTransform;
        }

        if (entity.HasComponent<RectTransformComponent>())
        {
            float targetWidth = static_cast<float>(m_ViewportWidth);
            float targetHeight = static_cast<float>(m_ViewportHeight);
            Entity canvasEntity = FindCanvasEntity();
            if ((targetWidth <= 0.0f || targetHeight <= 0.0f)
                && canvasEntity && canvasEntity.HasComponent<CanvasComponent>())
            {
                const auto& canvas = canvasEntity.GetComponent<CanvasComponent>();
                targetWidth = canvas.ReferenceResolution.x;
                targetHeight = canvas.ReferenceResolution.y;
            }
            return ResolveRectTransform(entity, targetWidth, targetHeight).WorldTransform;
        }

        return glm::mat4(1.0f);
    }

    glm::vec3 Scene::GetEntityWorldTranslation(Entity entity) const
    {
        glm::vec3 translation{};
        glm::vec3 rotation{};
        glm::vec3 scale{};
        Math::DecomposeTransform(GetEntityWorldTransformMatrix(entity), translation, rotation, scale);
        return translation;
    }

    glm::vec3 Scene::GetEntityWorldRotation(Entity entity) const
    {
        glm::vec3 translation{};
        glm::vec3 rotation{};
        glm::vec3 scale{};
        Math::DecomposeTransform(GetEntityWorldTransformMatrix(entity), translation, rotation, scale);
        return rotation;
    }

    glm::vec3 Scene::GetEntityWorldScale(Entity entity) const
    {
        glm::vec3 translation{};
        glm::vec3 rotation{};
        glm::vec3 scale{};
        Math::DecomposeTransform(GetEntityWorldTransformMatrix(entity), translation, rotation, scale);
        return scale;
    }

    void Scene::ApplyMatrixAsLocalTransform(Entity entity, const glm::mat4& localMatrix)
    {
        glm::vec3 translation{};
        glm::vec3 rotation{};
        glm::vec3 scale{};
        Math::DecomposeTransform(localMatrix, translation, rotation, scale);

        if (entity.HasComponent<TransformComponent>())
        {
            auto& transform = entity.GetComponent<TransformComponent>();
            transform.Position = translation;
            transform.Rotation = rotation;
            transform.Scale = scale;
        }
        else if (entity.HasComponent<RectTransformComponent>())
        {
            if (entity.HasComponent<CanvasComponent>())
            {
                // Canvas 根 Position/Rotation 锁定，忽略外部写回。
                SyncCanvasReferenceResolutionToTransform(entity);
                return;
            }

            auto& userInterfaceTransform = entity.GetComponent<RectTransformComponent>();
            userInterfaceTransform.AnchoredPosition = glm::vec2(translation);
            userInterfaceTransform.RotationRadians = rotation.z;
            // Size 不参与父子矩阵，分解出的 scale 不写回 Size（由编辑器 Gizmo 单独处理）。
        }
    }

    void Scene::ApplyWorldMatrixAsLocalTransform(Entity entity, const glm::mat4& worldMatrix)
    {
        Entity parentEntity = GetParentEntity(entity);
        const glm::mat4 parentWorldMatrix =
                parentEntity ? GetEntityWorldTransformMatrix(parentEntity) : glm::mat4(1.0f);
        ApplyMatrixAsLocalTransform(entity, glm::inverse(parentWorldMatrix) * worldMatrix);
    }

    void Scene::MarkEntityTransformDirty(Entity entity)
    {
        if (!entity)
            return;

        if (entity.HasComponent<TransformComponent>())
            entity.GetComponent<TransformComponent>().WorldTransformDirty = true;
        if (entity.HasComponent<RectTransformComponent>())
            entity.GetComponent<RectTransformComponent>().WorldTransformDirty = true;

        for (UUID childIdentifier : GetEntityChildren(entity))
        {
            Entity childEntity = GetEntityByUUID(childIdentifier);
            MarkEntityTransformDirty(childEntity);
        }
    }

    void Scene::NotifyEntityLocalTransformChanged(Entity entity)
    {
        MarkEntityTransformDirty(entity);
        SyncEntityTransformSubtreeToPhysics(entity);
    }

    void Scene::SyncEntityTransformSubtreeToPhysics(Entity entity)
    {
        if (!entity)
            return;

        if (entity.HasComponent<Rigidbody2DComponent>())
            SyncEntityTransformToPhysics(entity);

        for (UUID childIdentifier : GetEntityChildren(entity))
        {
            Entity childEntity = GetEntityByUUID(childIdentifier);
            SyncEntityTransformSubtreeToPhysics(childEntity);
        }
    }

    void Scene::SetEntityWorldTransformFromPhysics(Entity entity, const glm::vec2& worldPosition,
                                                   float worldRotationZ)
    {
        if (!entity || !entity.HasComponent<TransformComponent>())
            return;

        Entity parentEntity = GetParentEntity(entity);
        if (!parentEntity)
        {
            auto& transform = entity.GetComponent<TransformComponent>();
            transform.Position.x = worldPosition.x;
            transform.Position.y = worldPosition.y;
            transform.Rotation.z = worldRotationZ;
        }
        else
        {
            glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(worldPosition.x, worldPosition.y, 0.0f))
                                      * glm::rotate(glm::mat4(1.0f), worldRotationZ, glm::vec3(0.0f, 0.0f, 1.0f));
            ApplyWorldMatrixAsLocalTransform(entity, worldMatrix);
        }

        MarkEntityTransformDirty(entity);
    }

    bool Scene::SetEntityParent(Entity child, Entity parent, bool keepWorldPosition)
    {
        if (!child)
            return false;
        if (child == parent)
            return false;
        if (parent && IsEntityDescendantOf(parent, child))
            return false;
        if (parent && !EntitiesShareTransformDomain(child, parent))
            return false;

        glm::mat4 worldMatrixBefore = glm::mat4(1.0f);
        if (keepWorldPosition)
            worldMatrixBefore = GetEntityWorldTransformMatrix(child);

        if (!parent)
        {
            if (child.HasComponent<RelationshipComponent>())
                child.RemoveComponent<RelationshipComponent>();
        }
        else
        {
            if (!child.HasComponent<RelationshipComponent>())
                child.AddComponent<RelationshipComponent>();
            auto& relationship = child.GetComponent<RelationshipComponent>();
            relationship.Parent = parent.GetUUID();
            relationship.SiblingIndex =
                    static_cast<uint32_t>(GetEntityChildren(parent).size());
        }

        RebuildHierarchyCache();
        MarkEntityTransformDirty(child);

        if (keepWorldPosition)
        {
            const glm::mat4 parentWorldMatrix =
                    parent ? GetEntityWorldTransformMatrix(parent) : glm::mat4(1.0f);
            ApplyMatrixAsLocalTransform(child, glm::inverse(parentWorldMatrix) * worldMatrixBefore);
        }

        MarkEntityTransformDirty(child);
        SyncEntityTransformSubtreeToPhysics(child);
        return true;
    }

    void Scene::UnparentEntity(Entity child, bool keepWorldPosition)
    {
        SetEntityParent(child, {}, keepWorldPosition);
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
        (void)entity;
        (void)component;
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
    void Scene::OnComponentAdded<RectTransformComponent>(
            Entity entity, RectTransformComponent &component)
    {
        (void)entity;
        (void)component;
    }
    template<>
    void Scene::OnComponentAdded<UIImageComponent>(Entity entity, UIImageComponent &component)
    {
    }
    template<>
    void Scene::OnComponentAdded<UITextComponent>(Entity entity, UITextComponent &component)
    {
        if (!component.FontAsset)
            component.FontAsset = Font::GetDefault();
    }
    template<>
    void Scene::OnComponentAdded<UIButtonComponent>(Entity entity, UIButtonComponent &component)
    {
        (void)entity;
        (void)component;
    }
    template<>
    void Scene::OnComponentAdded<CanvasComponent>(Entity entity, CanvasComponent &component)
    {
        (void)component;
        SyncCanvasReferenceResolutionToTransform(entity);
    }
    template<>
    void Scene::OnComponentAdded<RelationshipComponent>(Entity entity, RelationshipComponent &component)
    {
        (void)entity;
        (void)component;
    }

} // namespace Himii
