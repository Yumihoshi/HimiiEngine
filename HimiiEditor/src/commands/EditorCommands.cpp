#include "EditorCommands.h"

namespace Himii
{
    CreateEntityCommand::CreateEntityCommand(const Ref<Scene>& scene,
                                             CreateEntityFunction createEntityFunction)
        : m_Scene(scene)
        , m_CreateEntityFunction(std::move(createEntityFunction))
    {
    }

    void CreateEntityCommand::Execute()
    {
        if (!m_EntitySnapshotYaml.empty())
        {
            EntitySnapshot::Restore(m_Scene, m_EntitySnapshotYaml);
            return;
        }

        Entity createdEntity = m_CreateEntityFunction(m_Scene);
        if (createdEntity)
            m_EntityIdentifier = createdEntity.GetUUID();
    }

    void CreateEntityCommand::Undo()
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityIdentifier);
        if (!entity)
            return;

        m_EntitySnapshotYaml = EntitySnapshot::Serialize(entity);
        m_Scene->DestroyEntity(entity);
    }

    DeleteEntityCommand::DeleteEntityCommand(const Ref<Scene>& scene, Entity entity,
                                             std::function<void(Entity)> onEntityRestored)
        : m_Scene(scene)
        , m_EntityIdentifier(entity.GetUUID())
        , m_OnEntityRestored(std::move(onEntityRestored))
    {
        m_EntitySnapshotYaml = EntitySnapshot::Serialize(entity);
    }

    void DeleteEntityCommand::Execute()
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityIdentifier);
        if (entity)
            m_Scene->DestroyEntity(entity);
    }

    void DeleteEntityCommand::Undo()
    {
        Entity restoredEntity = EntitySnapshot::Restore(m_Scene, m_EntitySnapshotYaml);
        if (restoredEntity && m_OnEntityRestored)
            m_OnEntityRestored(restoredEntity);
    }

    ModifyTransformCommand::ModifyTransformCommand(const Ref<Scene>& scene, UUID entityIdentifier,
                                                   const TransformComponent& beforeTransform,
                                                   const TransformComponent& afterTransform)
        : m_Scene(scene)
        , m_EntityIdentifier(entityIdentifier)
        , m_BeforeTransform(beforeTransform)
        , m_AfterTransform(afterTransform)
    {
    }

    void ModifyTransformCommand::ApplyTransform(const TransformComponent& transform)
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityIdentifier);
        if (entity && entity.HasComponent<TransformComponent>())
            entity.GetComponent<TransformComponent>() = transform;
    }

    void ModifyTransformCommand::Execute()
    {
        ApplyTransform(m_AfterTransform);
    }

    void ModifyTransformCommand::Undo()
    {
        ApplyTransform(m_BeforeTransform);
    }

    bool ModifyTransformCommand::TryMerge(const IEditorCommand& other)
    {
        const auto* otherCommand = dynamic_cast<const ModifyTransformCommand*>(&other);
        if (!otherCommand || otherCommand->m_EntityIdentifier != m_EntityIdentifier)
            return false;

        m_AfterTransform = otherCommand->m_AfterTransform;
        return true;
    }

    ModifyUITransformCommand::ModifyUITransformCommand(const Ref<Scene>& scene, UUID entityIdentifier,
                                                       const UITransformComponent& beforeTransform,
                                                       const UITransformComponent& afterTransform)
        : m_Scene(scene)
        , m_EntityIdentifier(entityIdentifier)
        , m_BeforeTransform(beforeTransform)
        , m_AfterTransform(afterTransform)
    {
    }

    void ModifyUITransformCommand::ApplyTransform(const UITransformComponent& transform)
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityIdentifier);
        if (entity && entity.HasComponent<UITransformComponent>())
            entity.GetComponent<UITransformComponent>() = transform;
    }

    void ModifyUITransformCommand::Execute()
    {
        ApplyTransform(m_AfterTransform);
    }

    void ModifyUITransformCommand::Undo()
    {
        ApplyTransform(m_BeforeTransform);
    }

    bool ModifyUITransformCommand::TryMerge(const IEditorCommand& other)
    {
        const auto* otherCommand = dynamic_cast<const ModifyUITransformCommand*>(&other);
        if (!otherCommand || otherCommand->m_EntityIdentifier != m_EntityIdentifier)
            return false;

        m_AfterTransform = otherCommand->m_AfterTransform;
        return true;
    }

    ModifyEntityTagCommand::ModifyEntityTagCommand(const Ref<Scene>& scene, UUID entityIdentifier,
                                                   std::string beforeTag, std::string afterTag)
        : m_Scene(scene)
        , m_EntityIdentifier(entityIdentifier)
        , m_BeforeTag(std::move(beforeTag))
        , m_AfterTag(std::move(afterTag))
    {
    }

    void ModifyEntityTagCommand::ApplyTag(const std::string& tag)
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityIdentifier);
        if (entity && entity.HasComponent<TagComponent>())
            entity.GetComponent<TagComponent>().Tag = tag;
    }

    void ModifyEntityTagCommand::Execute()
    {
        ApplyTag(m_AfterTag);
    }

    void ModifyEntityTagCommand::Undo()
    {
        ApplyTag(m_BeforeTag);
    }
}
