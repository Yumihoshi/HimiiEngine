#include "SceneHierarchyPanel.h"
#include "commands/EditorCommandHistory.h"
#include "commands/EditorCommands.h"
#include "Himii/Project/Project.h"

#include "Himii/Scene/PrefabSerializer.h"
#include "Himii/Utils/PlatformUtils.h"
#include "Himii/Asset/AssetManager.h"

#include "panel/ComponentInspector/ComponentInspectorDrawContext.h"
#include "panel/ComponentInspector/ComponentInspectorRegistry.h"

#include <imgui.h>
#include <filesystem>
#include <cstdio>

namespace Himii
{
    SceneHierarchyPanel::SceneHierarchyPanel()
    {
        // Load Component Icons
        m_ComponentIcons["Transform"] = Texture2D::Create("resources/icons/Component_Transform.png");
        m_ComponentIcons["Camera"] = Texture2D::Create("resources/icons/Component_Camera.png");
        m_ComponentIcons["Script"] = Texture2D::Create("resources/icons/Component_Script.png");
        m_ComponentIcons["Sprite Renderer"] = Texture2D::Create("resources/icons/Component_SpriteRenderer.png");
        m_ComponentIcons["Circle Renderer"] = Texture2D::Create("resources/icons/Component_CircleRenderer.png");
        m_ComponentIcons["Rigidbody2D"] = Texture2D::Create("resources/icons/Component_Rigidbody.png");
        m_ComponentIcons["Box Collider2D"] = Texture2D::Create("resources/icons/Component_BoxCollider.png");
        m_ComponentIcons["Circle Collider2D"] = Texture2D::Create("resources/icons/Component_CircleCollider.png");
        m_ComponentIcons["Sprite Animation"] = Texture2D::Create("resources/icons/Component_Animator.png");
        // Using SpriteRenderer icon as placeholder, or generate new one. For now re-use or use nullptr
        m_ComponentIcons["Mesh Renderer"] = Texture2D::Create("resources/icons/Component_SpriteRenderer.png"); 
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
        : SceneHierarchyPanel()
    {
        SetContext(context);
    }
    void SceneHierarchyPanel::SetContext(const Ref<Scene> &context)
    {
        m_Context = context;
    }

    void SceneHierarchyPanel::SetCommandHistory(EditorCommandHistory* commandHistory)
    {
        m_CommandHistory = commandHistory;
    }
    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse);

        if (m_Context)
        {
            const auto &view = m_Context->m_Registry.view<TagComponent>();
            for (auto entity: view)
            {
                Entity e{entity, m_Context.get()};
                DrawEntityNode(e);
            }
        }

        if (ImGui::BeginPopupContextWindow(0, 1))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
            {
                if (m_CommandHistory)
                {
                    m_CommandHistory->Execute(std::make_unique<CreateEntityCommand>(
                        m_Context,
                        [](const Ref<Scene>& scene) { return scene->CreateEntity("Empty Entity"); }));
                }
                else
                    m_Context->CreateEntity("Empty Entity");
            }
            if (ImGui::MenuItem("Create UI Entty"))
            {
                if (m_CommandHistory)
                {
                    m_CommandHistory->Execute(std::make_unique<CreateEntityCommand>(
                        m_Context,
                        [](const Ref<Scene>& scene) { return scene->CreateUIEntity("Empty UI Entity"); }));
                }
                else
                    m_Context->CreateUIEntity("Empty UI Entity");
            }
            if (ImGui::MenuItem("Create Cube"))
            {
                if (m_CommandHistory)
                {
                    m_CommandHistory->Execute(std::make_unique<CreateEntityCommand>(
                        m_Context,
                        [](const Ref<Scene>& scene)
                        {
                            Entity entity = scene->CreateEntity("Cube");
                            entity.AddComponent<MeshComponent>().Type = MeshComponent::MeshType::Cube;
                            return entity;
                        }));
                }
                else
                {
                    auto entity = m_Context->CreateEntity("Cube");
                    entity.AddComponent<MeshComponent>().Type = MeshComponent::MeshType::Cube;
                }
            }
            if (ImGui::MenuItem("Create Sphere"))
            {
                if (m_CommandHistory)
                {
                    m_CommandHistory->Execute(std::make_unique<CreateEntityCommand>(
                        m_Context,
                        [](const Ref<Scene>& scene)
                        {
                            Entity entity = scene->CreateEntity("Sphere");
                            entity.AddComponent<MeshComponent>().Type = MeshComponent::MeshType::Sphere;
                            return entity;
                        }));
                }
                else
                {
                    auto entity = m_Context->CreateEntity("Sphere");
                    entity.AddComponent<MeshComponent>().Type = MeshComponent::MeshType::Sphere;
                }
            }
            if (ImGui::MenuItem("Create Capsule"))
            {
                if (m_CommandHistory)
                {
                    m_CommandHistory->Execute(std::make_unique<CreateEntityCommand>(
                        m_Context,
                        [](const Ref<Scene>& scene)
                        {
                            Entity entity = scene->CreateEntity("Capsule");
                            entity.AddComponent<MeshComponent>().Type = MeshComponent::MeshType::Capsule;
                            return entity;
                        }));
                }
                else
                {
                    auto entity = m_Context->CreateEntity("Capsule");
                    entity.AddComponent<MeshComponent>().Type = MeshComponent::MeshType::Capsule;
                }
            }
            ImGui::EndPopup();
        }
        ImGui::End();

        ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoCollapse);

        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);

            if (ImGui::BeginPopupContextWindow(0, 1))
            {
                DisplayAddComponentEntry<CameraComponent>("Camera");
                DisplayAddComponentEntry<ScriptComponent>("Script Component");
                DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
                DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
                DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody2D");
                DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider2D");
                DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider2D");
                DisplayAddComponentEntry<MeshComponent>("Mesh Renderer");
                DisplayAddComponentEntry<TilemapComponent>("Tilemap");
                DisplayAddComponentEntry<TilemapCollider2DComponent>("Tilemap Collider 2D");
                DisplayAddComponentEntry<ParticleEmitterComponent>("Particle Emitter");
                DisplayAddComponentEntry<UITransformComponent>("Rect Transform");
                DisplayAddComponentEntry<UIImageComponent>("Image");

                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto &tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags =
                ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        flags |= ImGuiTreeNodeFlags_Leaf; // Fix: Treat as leaf node preventing arrow
        bool opened = ImGui::TreeNodeEx((void *)(uint64_t)(uint32_t)entity, flags, tag.c_str());
        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Save as Prefab..."))
            {
                std::string filePath = FileDialog::SaveFile("Himii Prefab (*.hprefab)\0*.hprefab\0");
                if (!filePath.empty())
                {
                    if (PrefabSerializer::Save(entity, filePath))
                    {
                        if (Project::GetActive())
                        {
                            std::filesystem::path relativePath =
                                    std::filesystem::relative(filePath, Project::GetAssetDirectory());
                            if (auto assetManager = Project::GetAssetManager())
                                assetManager->ImportAsset(relativePath);
                        }
                    }
                }
            }

            if (ImGui::MenuItem("Delete Entity"))
            {
                entityDeleted = true;
            }
            ImGui::EndPopup();
        }

        if (opened)
        {
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            if (m_CommandHistory)
            {
                auto onEntityRestored = [this](Entity restoredEntity)
                {
                    m_SelectionContext = restoredEntity;
                };
                m_CommandHistory->Execute(
                    std::make_unique<DeleteEntityCommand>(m_Context, entity, onEntityRestored));
            }
            else
                m_Context->DestroyEntity(entity);

            if (m_SelectionContext == entity)
                m_SelectionContext = {};
        }
    }
    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto &tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));

            strcat(buffer, tag.c_str());
            std::string tagBeforeInput = tag;
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
                tag = std::string(buffer);

            if (ImGui::IsItemActivated())
                m_TagEditStartValue = tagBeforeInput;

            if (ImGui::IsItemDeactivatedAfterEdit() && m_CommandHistory)
            {
                if (tag != m_TagEditStartValue)
                {
                    m_CommandHistory->Execute(std::make_unique<ModifyEntityTagCommand>(
                        m_Context, entity.GetUUID(), m_TagEditStartValue, tag));
                }
            }
        }
        ImGui::SameLine();
        // ImGui::PushItemWidth(-1);
        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            DisplayAddComponentEntry<CameraComponent>("Camera");
            DisplayAddComponentEntry<ScriptComponent>("Script Component");
            DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
            DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
            DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody2D");
            DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider2D");
            DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider2D");
            DisplayAddComponentEntry<SpriteAnimationComponent>("Sprite Animation");
            DisplayAddComponentEntry<MeshComponent>("Mesh Renderer");
            DisplayAddComponentEntry<TilemapComponent>("Tilemap");
            DisplayAddComponentEntry<TilemapCollider2DComponent>("Tilemap Collider 2D");
            DisplayAddComponentEntry<ParticleEmitterComponent>("Particle Emitter");

            //UI
            DisplayAddComponentEntry<UITransformComponent>("Rect Transform");
            DisplayAddComponentEntry<UIImageComponent>("Image");
            DisplayAddComponentEntry<UITextComponent>("Text");

            ImGui::EndPopup();
        }

        ComponentInspectorDrawContext componentInspectorDrawContext;
        componentInspectorDrawContext.scene = m_Context;
        componentInspectorDrawContext.entity = entity;
        componentInspectorDrawContext.commandHistory = m_CommandHistory;

        componentInspectorDrawContext.getComponentIcon = [this](const std::string& iconKey)
        {
            auto iterator = m_ComponentIcons.find(iconKey);
            if (iterator != m_ComponentIcons.end())
                return iterator->second;
            return Ref<Texture2D>{};
        };

        componentInspectorDrawContext.requestTextureInspector = [this](AssetHandle handle)
        {
            m_TextureInspectorRequest = handle;
        };

        componentInspectorDrawContext.requestParticleEmitterEditor = [this](AssetHandle handle)
        {
            m_ParticleEmitterEditorRequest = handle;
        };

        componentInspectorDrawContext.requestAnimationEditor =
            [this](const std::filesystem::path& animationEditorPath)
        {
            m_AnimationEditorRequest = animationEditorPath;
        };

        componentInspectorDrawContext.requestTileMapEditor = [this](AssetHandle handle)
        {
            m_TileMapEditorRequest = handle;
        };

        ComponentInspectorRegistry::Get().DrawAll(componentInspectorDrawContext);
    }
    template<typename T>
    void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string &entryName)
    {
        if (!m_SelectionContext.HasComponent<T>())
        {
            if (ImGui::MenuItem(entryName.c_str()))
            {
                m_SelectionContext.AddComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }

} // namespace Himii
