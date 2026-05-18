#pragma once

#include "IEditorCommand.h"
#include "EntitySnapshot.h"
#include "Engine.h"
#include <functional>

namespace Himii
{
    class CreateEntityCommand : public IEditorCommand
    {
    public:
        using CreateEntityFunction = std::function<Entity(Ref<Scene>)>;

        CreateEntityCommand(const Ref<Scene>& scene, CreateEntityFunction createEntityFunction);

        void Execute() override;
        void Undo() override;

    private:
        Ref<Scene> m_Scene;
        CreateEntityFunction m_CreateEntityFunction;
        UUID m_EntityIdentifier = 0;
        std::string m_EntitySnapshotYaml;
    };

    class DeleteEntityCommand : public IEditorCommand
    {
    public:
        DeleteEntityCommand(const Ref<Scene>& scene, Entity entity,
                            std::function<void(Entity)> onEntityRestored = nullptr);

        void Execute() override;
        void Undo() override;

    private:
        Ref<Scene> m_Scene;
        UUID m_EntityIdentifier = 0;
        std::string m_EntitySnapshotYaml;
        std::function<void(Entity)> m_OnEntityRestored;
    };

    class ModifyTransformCommand : public IEditorCommand
    {
    public:
        ModifyTransformCommand(const Ref<Scene>& scene, UUID entityIdentifier,
                               const TransformComponent& beforeTransform,
                               const TransformComponent& afterTransform);

        void Execute() override;
        void Undo() override;
        bool TryMerge(const IEditorCommand& other) override;

    private:
        void ApplyTransform(const TransformComponent& transform);

        Ref<Scene> m_Scene;
        UUID m_EntityIdentifier;
        TransformComponent m_BeforeTransform;
        TransformComponent m_AfterTransform;
    };

    class ModifyUITransformCommand : public IEditorCommand
    {
    public:
        ModifyUITransformCommand(const Ref<Scene>& scene, UUID entityIdentifier,
                                 const UITransformComponent& beforeTransform,
                                 const UITransformComponent& afterTransform);

        void Execute() override;
        void Undo() override;
        bool TryMerge(const IEditorCommand& other) override;

    private:
        void ApplyTransform(const UITransformComponent& transform);

        Ref<Scene> m_Scene;
        UUID m_EntityIdentifier;
        UITransformComponent m_BeforeTransform;
        UITransformComponent m_AfterTransform;
    };

    class ModifyEntityTagCommand : public IEditorCommand
    {
    public:
        ModifyEntityTagCommand(const Ref<Scene>& scene, UUID entityIdentifier,
                               std::string beforeTag, std::string afterTag);

        void Execute() override;
        void Undo() override;

    private:
        void ApplyTag(const std::string& tag);

        Ref<Scene> m_Scene;
        UUID m_EntityIdentifier;
        std::string m_BeforeTag;
        std::string m_AfterTag;
    };
}
