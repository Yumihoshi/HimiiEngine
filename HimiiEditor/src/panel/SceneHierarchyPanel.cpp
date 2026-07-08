#include "SceneHierarchyPanel.h"
#include "InspectorControls.h"
#include "commands/EditorCommandHistory.h"
#include "commands/EditorCommands.h"
#include "Himii/Project/Project.h"

#include "Himii/Scripting/ScriptEngine.h"
#include "Himii/Scripting/ScriptIDELauncher.h"
#include "Himii/Scene/TileSet.h"
#include "Himii/Scene/TileMapData.h"
#include "Himii/Scene/TilemapColliderBuilder.h"
#include "Himii/Scene/ParticleEmitterAsset.h"
#include "Himii/Scene/SpriteAnimation.h"
#include "Himii/Scene/SpriteAnimationUtility.h"
#include "Himii/Scene/PrefabSerializer.h"
#include "Himii/Utils/PlatformUtils.h"
#include "Himii/Asset/AssetSerializer.h"
#include "Himii/Asset/AssetManager.h"
#include "Himii/Asset/Sprite.h"
#include "Himii/Core/Log.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>
#include <cstdio>

#include <glm/gtc/type_ptr.hpp>

namespace Himii
{
    extern const std::filesystem::path s_AssetsPath;

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

    static bool IsImageFileExtension(const std::filesystem::path& filePath)
    {
        std::string extension = filePath.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
        return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp"
               || extension == ".tga";
    }

    static void DrawTextureAssignControl(const char* label, Ref<Texture2D>& texture, AssetHandle& textureHandle)
    {
        const char* displayName = "None";
        std::string displayNameStorage;
        if (texture)
        {
            displayNameStorage = std::filesystem::path(texture->GetPath()).filename().string();
            displayName = displayNameStorage.c_str();
        }

        DrawObjectReferenceField(
            label, displayName, textureHandle != 0, texture,
            [&]()
            {
                texture = nullptr;
                textureHandle = 0;
            },
            [&](const ImGuiPayload* payload)
            {
                return AssignTextureFromContentBrowserPayload(payload, texture, textureHandle);
            });
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string &name, Entity entity, Ref<Texture2D> icon, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                 ImGuiTreeNodeFlags_SpanAvailWidth |
                                                 ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        if (entity.HasComponent<T>())
        {
            auto &component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
            float lineHeight = GImGui->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            
        // --- Custom Icon Header ---
            ImGui::PushID(name.c_str());
            
            bool open = ImGui::GetStateStorage()->GetInt(ImGui::GetID("IsOpen"), 1);

            // Calculate start pos
            ImVec2 cursorPos = ImGui::GetCursorScreenPos(); // Screen Space
            ImVec2 elementPos = ImGui::GetCursorPos();      // Window Space (for restoring)
            
            // 1. Draw Background (Lighter color)
            ImU32 headerColor = ImGui::GetColorU32(ImVec4{0.16f, 0.16f, 0.16f, 1.0f});
            ImGui::GetWindowDrawList()->AddRectFilled(
                cursorPos, 
                ImVec2(cursorPos.x + contentRegionAvailable.x, cursorPos.y + lineHeight), 
                headerColor
            );

            // 2. Draw Selectable (Input handling)
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0,0,0,0)); 
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0,0,0,0)); 
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0,0,0,0)); 

            if (ImGui::Selectable("##Header", false, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(contentRegionAvailable.x, lineHeight)))
            {
                open = !open;
                ImGui::GetStateStorage()->SetInt(ImGui::GetID("IsOpen"), open ? 1 : 0);
            }
            ImGui::PopStyleColor(3);

            // 3. Draw Arrow & Icon & Text (Overlay)
            // Reset cursor to start of the item to draw on top
            ImGui::SetCursorPos(ImVec2(elementPos.x + ImGui::GetStyle().WindowPadding.x, elementPos.y));

            // Arrow
            ImGui::RenderArrow(ImGui::GetWindowDrawList(), 
                ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + (lineHeight - GImGui->FontSize)/2), 
                ImGui::GetColorU32(ImGuiCol_Text), 
                open ? ImGuiDir_Down : ImGuiDir_Right
            );
            
            // Move cursor past arrow
            ImGui::SetCursorPos(ImVec2(elementPos.x + ImGui::GetStyle().WindowPadding.x + GImGui->FontSize + 4.0f, elementPos.y));

            // Icon
            if (icon)
            {
                ImGui::Image((ImTextureID)icon->GetRendererID(), ImVec2{lineHeight - 4, lineHeight - 4}, {0, 1}, {1, 0});
                ImGui::SameLine();
            }
            
            // Text alignment fix (center vertically roughly)
            float textOffsetY = (lineHeight - GImGui->FontSize) / 2.0f;
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textOffsetY);
            ImGui::TextUnformatted(name.c_str());
            
            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            ImGui::SetCursorPosY(elementPos.y);
            if (ImGui::Button("+", ImVec2{lineHeight, lineHeight}))
            {
                ImGui::OpenPopup("ComponentSetting");
            }
            ImGui::SetCursorPos(ImVec2(elementPos.x, elementPos.y + lineHeight));
            
            ImGui::PopStyleVar(); // Pop FramePadding after header and button

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSetting"))
            {
                if (ImGui::MenuItem("Remove component"))
                    removeComponent = true;
                ImGui::EndPopup();
            }

            if (open)
            {
                uiFunction(component);
            }

            if (removeComponent)
                entity.RemoveComponent<T>();
            
            ImGui::PopID(); // Pop Component ID at the very end
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

        const UUID entityIdentifier = entity.GetUUID();
        DrawComponent<TransformComponent>("Transform", entity, m_ComponentIcons["Transform"],
                                          [this, entityIdentifier](auto &component)
                                          {
                                              TransformComponent transformBeforeEdit = component;
                                              bool transformEditCaptured = false;

                                              auto beginTransformEdit = [&]()
                                              {
                                                  if (!transformEditCaptured)
                                                  {
                                                      transformBeforeEdit = component;
                                                      transformEditCaptured = true;
                                                  }
                                              };

                                              auto endTransformEdit = [&]()
                                              {
                                                  if (!transformEditCaptured || !m_CommandHistory)
                                                      return;

                                                  TransformComponent transformAfterEdit = component;
                                                  if (transformBeforeEdit.Position != transformAfterEdit.Position
                                                      || transformBeforeEdit.Rotation != transformAfterEdit.Rotation
                                                      || transformBeforeEdit.Scale != transformAfterEdit.Scale)
                                                  {
                                                      m_CommandHistory->Execute(
                                                          std::make_unique<ModifyTransformCommand>(
                                                              m_Context, entityIdentifier, transformBeforeEdit,
                                                              transformAfterEdit));
                                                  }
                                                  transformEditCaptured = false;
                                              };

                                              DrawVec3Control("Position", component.Position, 0.0f,
                                                              beginTransformEdit, endTransformEdit);
                                              glm::vec3 rotation = glm::degrees(component.Rotation);
                                              DrawVec3Control("Rotation", rotation, 0.0f,
                                                              beginTransformEdit, endTransformEdit);
                                              component.Rotation = glm::radians(rotation);
                                              DrawVec3Control("Scale", component.Scale, 1.0f,
                                                              beginTransformEdit, endTransformEdit);
                                          });

        DrawComponent<CameraComponent>("Camera", entity, m_ComponentIcons["Camera"],
                                       [](auto &component)
                                       {
                                           auto &camera = component.Camera;
                                           
                                           glm::vec4 backgroundColor = camera.GetBackgroundColor();
                                           const glm::vec4 previousBackgroundColor = backgroundColor;
                                           DrawColorControl("Background Color", backgroundColor);
                                           if (backgroundColor != previousBackgroundColor)
                                               camera.SetBackgroundColor(backgroundColor);

                                           DrawCheckboxControl("Primary", component.Primary);

                                           const char* projectionTypeStrings[] = {"Perspective", "Orthographic"};
                                           int projectionTypeIndex = static_cast<int>(camera.GetProjectionType());
                                           DrawEnumComboControl(
                                               "Projection", projectionTypeIndex, projectionTypeStrings, 2,
                                               [&](int newIndex)
                                               {
                                                   camera.SetProjectionType(
                                                       static_cast<SceneCamera::ProjectionType>(newIndex));
                                               });

                                           if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
                                           {
                                               float perspectiveFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
                                               DrawFloatControl("Vertical FOV", perspectiveFOV);
                                               if (perspectiveFOV != glm::degrees(camera.GetPerspectiveVerticalFOV()))
                                                   camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveFOV));

                                               float perspectiveNear = camera.GetPerspectiveNearClip();
                                               DrawFloatControl("Near", perspectiveNear);
                                               camera.SetPerspectiveNearClip(perspectiveNear);

                                               float perspectiveFar = camera.GetPerspectiveFarClip();
                                               DrawFloatControl("Far", perspectiveFar);
                                               camera.SetPerspectiveFarClip(perspectiveFar);
                                           }
                                           if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
                                           {
                                               float orthographicSize = camera.GetOrthographicSize();
                                               DrawFloatControl("Size", orthographicSize);
                                               camera.SetOrthographicSize(orthographicSize);

                                               float orthographicNear = camera.GetOrthographicNearClip();
                                               DrawFloatControl("Near", orthographicNear);
                                               camera.SetOrthographicNearClip(orthographicNear);

                                               float orthographicFar = camera.GetOrthographicFarClip();
                                               DrawFloatControl("Far", orthographicFar);
                                               camera.SetOrthographicFarClip(orthographicFar);

                                               DrawCheckboxControl("Fixed Aspect Ratio", component.FixedAspectRatio);
                                           }
                                       });

        DrawComponent<ScriptComponent>(
                "Script", entity, m_ComponentIcons["Script"],
                [entity, scene = m_Context](auto &component) mutable
                {
                    //bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

                    DrawPropertyRow("Class Name", [&]()
                    {
                        char classNameBuffer[256];
                        std::memset(classNameBuffer, 0, sizeof(classNameBuffer));
                        std::snprintf(classNameBuffer, sizeof(classNameBuffer), "%s", component.ClassName.c_str());

                        const bool scriptClassExistsForButton =
                            !component.ClassName.empty() && ScriptEngine::EntityClassExists(component.ClassName);
                        const float editButtonWidth =
                            ImGui::CalcTextSize("Edit").x + ImGui::GetStyle().FramePadding.x * 2.0f;
                        const float reservedWidth = scriptClassExistsForButton
                            ? editButtonWidth + ImGui::GetStyle().ItemSpacing.x
                            : 0.0f;

                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - reservedWidth);
                        ImGui::InputText("##ClassName", classNameBuffer, sizeof(classNameBuffer));
                        if (ImGui::IsItemDeactivatedAfterEdit())
                            component.ClassName = classNameBuffer;

                        if (scriptClassExistsForButton)
                        {
                            ImGui::SameLine();
                            if (ImGui::Button("Edit", ImVec2(editButtonWidth, 0.0f)))
                            {
                                if (auto project = Project::GetActive())
                                {
                                    auto projectDir = Project::GetProjectDirectory();
                                    std::filesystem::path scriptPath =
                                        projectDir / "assets" / "scripts" / (component.ClassName + ".cs");

                                    if (std::filesystem::exists(scriptPath))
                                    {
                                        ScriptIDELauncher::OpenScript(
                                            projectDir,
                                            project->GetConfig().Name,
                                            scriptPath);
                                    }
                                }
                            }
                        }
                    });

                    bool scriptClassExists = false;
                    if (!component.ClassName.empty())
                        scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

                    if (!scriptClassExists && !component.ClassName.empty())
                        ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Invalid Script Class!");
                    else if (scriptClassExists)
                    {
                        {
                             auto& fields = ScriptEngine::GetScriptFieldMap(entity);
                             
                             for (auto& [name, field] : fields)
                             {
                                 if (field.Type == ScriptFieldType::Float)
                                 {
                                     float data = field.GetValue<float>();
                                     float oldData = data;
                                     DrawFloatControl(name, data);
                                     if (data != oldData)
                                     {
                                         field.SetValue(data);
                                         ScriptEngine::SetFloat(ScriptEngine::GetEntityScriptInstance(entity.GetUUID()), name, data);
                                     }
                                 }
                                 else if (field.Type == ScriptFieldType::Int)
                                 {
                                     int data = field.GetValue<int>();
                                     int oldData = data;
                                     DrawIntControl(name.c_str(), data);
                                     if (data != oldData)
                                     {
                                         field.SetValue(data);
                                         ScriptEngine::SetInt(ScriptEngine::GetEntityScriptInstance(entity.GetUUID()), name, data);
                                     }
                                 }
                                 else if (field.Type == ScriptFieldType::Bool)
                                 {
                                     bool data = field.GetValue<bool>();
                                     bool oldData = data;
                                     DrawCheckboxControl(name, data);
                                     if (data != oldData)
                                     {
                                         field.SetValue(data);
                                         ScriptEngine::SetBool(ScriptEngine::GetEntityScriptInstance(entity.GetUUID()), name, data);
                                     }
                                 }
                                 else if (field.Type == ScriptFieldType::Vector2)
                                 {
                                     glm::vec2 data = field.GetValue<glm::vec2>();
                                     glm::vec2 oldData = data;
                                     DrawVec2Control(name.c_str(), data);
                                     if (data != oldData)
                                     {
                                         field.SetValue(data);
                                         ScriptEngine::SetVector2(ScriptEngine::GetEntityScriptInstance(entity.GetUUID()), name, data);
                                     }
                                 }
                                 else if (field.Type == ScriptFieldType::Vector3)
                                 {
                                     glm::vec3 data = field.GetValue<glm::vec3>();
                                     glm::vec3 oldData = data;
                                     DrawVec3Control(name, data);
                                     if (data != oldData)
                                     {
                                         field.SetValue(data);
                                         ScriptEngine::SetVector3(ScriptEngine::GetEntityScriptInstance(entity.GetUUID()), name, data);
                                     }
                                 }
                                 else if (field.Type == ScriptFieldType::Vector4)
                                 {
                                     glm::vec4 data = field.GetValue<glm::vec4>();
                                     glm::vec4 oldData = data;
                                     DrawColorControl(name, data);
                                     if (data != oldData)
                                     {
                                         field.SetValue(data);
                                         ScriptEngine::SetVector4(ScriptEngine::GetEntityScriptInstance(entity.GetUUID()), name, data);
                                     }
                                 }
                                else if (field.Type == ScriptFieldType::String)
                                {
                                    std::string data = field.GetValue<std::string>();
                                    std::string oldData = data;
                                    DrawStdStringControl(name.c_str(), data);
                                    if (data != oldData)
                                    {
                                        field.SetValue(data);
                                        ScriptEngine::SetString(ScriptEngine::GetEntityScriptInstance(entity.GetUUID()), name, data);
                                    }
                                }
                                else if (field.Type == ScriptFieldType::KeyCode)
                                {
                                    int data = field.GetValue<int>();

                                    struct KeyCodeOption { int Code; const char* Label; };
                                    static const KeyCodeOption keyCodeOptions[] = {
                                        { 32,  "Space" },
                                        { 65,  "A" },
                                        { 68,  "D" },
                                        { 83,  "S" },
                                        { 87,  "W" },
                                        { 262, "Right" },
                                        { 263, "Left" },
                                        { 264, "Down" },
                                        { 265, "Up" },
                                        { 257, "Enter" }
                                    };

                                    const char* keyCodeLabels[std::size(keyCodeOptions)] = {};
                                    for (size_t optionIndex = 0; optionIndex < std::size(keyCodeOptions); ++optionIndex)
                                        keyCodeLabels[optionIndex] = keyCodeOptions[optionIndex].Label;

                                    int selectedOptionIndex = 0;
                                    for (size_t optionIndex = 0; optionIndex < std::size(keyCodeOptions); ++optionIndex)
                                    {
                                        if (keyCodeOptions[optionIndex].Code == data)
                                        {
                                            selectedOptionIndex = static_cast<int>(optionIndex);
                                            break;
                                        }
                                    }

                                    DrawEnumComboControl(
                                        name.c_str(), selectedOptionIndex, keyCodeLabels,
                                        static_cast<int>(std::size(keyCodeOptions)),
                                        [&](int newIndex)
                                        {
                                            data = keyCodeOptions[newIndex].Code;
                                            field.SetValue(data);
                                            ScriptEngine::SetInt(
                                                ScriptEngine::GetEntityScriptInstance(entity.GetUUID()), name, data);
                                        });
                                }
                                else if (field.Type == ScriptFieldType::Entity)
                                {
                                    UUID data = field.GetValue<UUID>();
                                    DrawPropertyRow(name.c_str(), [&]()
                                    {
                                        std::string preview = "None";
                                        if (data)
                                        {
                                            Entity refEntity = scene->GetEntityByUUID(data);
                                            if (refEntity)
                                                preview = refEntity.GetName();
                                            else
                                                preview = "Missing";
                                        }

                                        ImGui::PushItemWidth(-1.0f);
                                        if (ImGui::BeginCombo("##EntityReference", preview.c_str()))
                                        {
                                            if (ImGui::Selectable("None", data == UUID{}))
                                            {
                                                data = {};
                                                field.SetValue(data);
                                                ScriptEngine::SetEntityField(
                                                    ScriptEngine::GetEntityScriptInstance(entity.GetUUID()), name, data);
                                            }

                                            if (scene)
                                            {
                                                const auto& view = scene->m_Registry.view<TagComponent>();
                                                for (auto sceneHandle : view)
                                                {
                                                    Entity sceneEntity{sceneHandle, scene.get()};
                                                    if (sceneEntity == entity)
                                                        continue;

                                                    UUID sceneUUID = sceneEntity.GetUUID();
                                                    const bool isSelected = sceneUUID == data;
                                                    if (ImGui::Selectable(sceneEntity.GetName().c_str(), isSelected))
                                                    {
                                                        data = sceneUUID;
                                                        field.SetValue(data);
                                                        ScriptEngine::SetEntityField(
                                                            ScriptEngine::GetEntityScriptInstance(entity.GetUUID()),
                                                            name, data);
                                                    }
                                                    if (isSelected)
                                                        ImGui::SetItemDefaultFocus();
                                                }
                                            }
                                            ImGui::EndCombo();
                                        }
                                        ImGui::PopItemWidth();
                                    });
                                }
                             }
                        }
                    }
                });

        DrawComponent<SpriteRendererComponent>(
                "Sprite Renderer", entity, m_ComponentIcons["Sprite Renderer"],
                [this](auto &component)
                {
                    DrawColorControl("Color", component.Color);

                    auto assetManager = Project::GetAssetManager();

                    Ref<Texture2D> previewTexture;
                    std::string displayNameStorage = "None";
                    const char* displayName = displayNameStorage.c_str();
                    AssetHandle parentTextureHandle = 0;

                    if (assetManager && component.SpriteAssetHandle != 0)
                    {
                        const SpriteDefinition* spriteDefinition =
                            assetManager->GetSpriteDefinition(component.SpriteAssetHandle);
                        if (spriteDefinition)
                            displayNameStorage = spriteDefinition->Name;

                        parentTextureHandle =
                            assetManager->GetTextureHandleForSprite(component.SpriteAssetHandle);
                        if (parentTextureHandle != 0)
                        {
                            Ref<Asset> textureAsset = assetManager->GetAsset(parentTextureHandle);
                            if (textureAsset)
                                previewTexture = std::static_pointer_cast<Texture2D>(textureAsset);

                            const std::filesystem::path texturePath =
                                assetManager->GetAssetRegistry().at(parentTextureHandle).FilePath;
                            displayNameStorage += " (";
                            displayNameStorage += texturePath.filename().string();
                            displayNameStorage += ")";
                        }
                        displayName = displayNameStorage.c_str();
                    }

                    DrawObjectReferenceField(
                        "Sprite", displayName, component.SpriteAssetHandle != 0, previewTexture,
                        [&]()
                        {
                            component.SpriteAssetHandle = 0;
                        },
                        [&](const ImGuiPayload* payload)
                        {
                            return AssignSpriteFromContentBrowserPayload(payload, component.SpriteAssetHandle);
                        },
                        [&]()
                        {
                            if (parentTextureHandle != 0)
                                m_TextureInspectorRequest = parentTextureHandle;
                        });

                    if (assetManager && parentTextureHandle != 0)
                    {
                        const std::vector<SpriteDefinition>& sprites =
                            assetManager->GetSpritesForTexture(parentTextureHandle);

                        if (sprites.size() > 1)
                        {
                            int currentSpriteIndex = 0;
                            std::vector<const char*> spriteLabels;
                            spriteLabels.reserve(sprites.size());
                            for (int spriteIndex = 0; spriteIndex < static_cast<int>(sprites.size()); ++spriteIndex)
                            {
                                spriteLabels.push_back(sprites[spriteIndex].Name.c_str());
                                if (sprites[spriteIndex].Handle == component.SpriteAssetHandle)
                                    currentSpriteIndex = spriteIndex;
                            }

                            DrawEnumComboControl(
                                "Variant", currentSpriteIndex, spriteLabels.data(),
                                static_cast<int>(spriteLabels.size()),
                                [&](int newIndex)
                                {
                                    if (newIndex >= 0 && newIndex < static_cast<int>(sprites.size()))
                                        component.SpriteAssetHandle = sprites[newIndex].Handle;
                                });
                        }
                    }

                    DrawFloatControl("Tiling Factor", component.TilingFactor, 0.1f, 0.0f, 100.0f);
                    DrawCheckboxControl("Flip Horizontal", component.FlipHorizontal);

                    component.SortingLayer =
                        std::clamp(component.SortingLayer, 0, SortingLayerCount - 1);
                    int sortingLayerIndex = component.SortingLayer;

                    SortingLayerSettings sortingLayerSettings;
                    if (Project::GetActive())
                        sortingLayerSettings = Project::GetConfig().SortingLayers;

                    std::vector<const char*> sortingLayerLabels;
                    sortingLayerLabels.reserve(SortingLayerCount);
                    for (int layerIndex = 0; layerIndex < SortingLayerCount; ++layerIndex)
                        sortingLayerLabels.push_back(
                            sortingLayerSettings.LayerNames[layerIndex].c_str());

                    DrawEnumComboControl(
                        "Sorting Layer", sortingLayerIndex, sortingLayerLabels.data(),
                        SortingLayerCount,
                        [&](int newIndex)
                        {
                            component.SortingLayer = newIndex;
                        });

                    DrawIntControl("Sorting Order", component.SortingOrder);
                });

        DrawComponent<MeshComponent>(
                 "Mesh Renderer", entity, m_ComponentIcons["Mesh Renderer"],
                 [](auto &component)
                 {
                     const char* meshTypeStrings[] = { "Cube", "Plane", "Sphere", "Capsule" };
                     int meshTypeIndex = static_cast<int>(component.Type);
                     DrawEnumComboControl(
                         "Mesh Type", meshTypeIndex, meshTypeStrings, 4,
                         [&](int newIndex)
                         {
                             component.Type = static_cast<MeshComponent::MeshType>(newIndex);
                         });

                     DrawColorControl("Color", component.Color);
                 });

        DrawComponent<CircleRendererComponent>("Circle Renderer", entity, m_ComponentIcons["Circle Renderer"],
                                               [](auto &component)
                                               {
                                                   DrawColorControl("Color", component.Color);
                                                   DrawFloatControl("Thickness", component.Thickness, 0.025f, 0.0f, 1.0f);
                                                   DrawFloatControl("Fade", component.Fade, 0.0003f, 0.0f, 1.0f);
                                               });

        DrawComponent<Rigidbody2DComponent>("Rigidbody2D", entity, m_ComponentIcons["Rigidbody2D"],
                                            [](auto &component)
                                            {
                                                const char* bodyTypeStrings[] = {"Static", "Dynamic", "Kinematic"};
                                                int bodyTypeIndex = static_cast<int>(component.Type);
                                                DrawEnumComboControl(
                                                    "Body Type", bodyTypeIndex, bodyTypeStrings, 3,
                                                    [&](int newIndex)
                                                    {
                                                        component.Type =
                                                            static_cast<Rigidbody2DComponent::BodyType>(newIndex);
                                                    });

                                                DrawCheckboxControl("Fixed Rotation", component.FixedRotation);
                                            });

        DrawComponent<BoxCollider2DComponent>(
                "Box Collider2D", entity, m_ComponentIcons["Box Collider2D"],
                [](auto &component)
                {
                    DrawVec2Control("Offset", component.Offset, 0.1f);
                    DrawVec2Control("Size", component.Size, 0.1f);
                    DrawFloatControl("Density", component.Density, 0.1f);
                    DrawFloatControl("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
                    DrawFloatControl("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
                    DrawFloatControl("Restitution Threshold", component.RestitutionThreshold, 0.1f);
                    DrawCheckboxControl("Is Trigger", component.IsTrigger);
                    component.Layer = std::clamp(component.Layer, 0, Physics2DLayerCount - 1);

                    int physicsLayerIndex = component.Layer;
                    Physics2DLayerSettings physicsLayerSettings;
                    if (Project::GetActive())
                        physicsLayerSettings = Project::GetConfig().Physics2DLayers;

                    std::vector<const char*> physicsLayerLabels;
                    physicsLayerLabels.reserve(Physics2DLayerCount);
                    for (int layerIndex = 0; layerIndex < Physics2DLayerCount; ++layerIndex)
                        physicsLayerLabels.push_back(
                            physicsLayerSettings.LayerNames[layerIndex].c_str());

                    DrawEnumComboControl(
                        "Layer", physicsLayerIndex, physicsLayerLabels.data(),
                        Physics2DLayerCount,
                        [&](int newIndex)
                        {
                            component.Layer = newIndex;
                        });
                });
        DrawComponent<CircleCollider2DComponent>(
                "Circle Collider2D", entity, m_ComponentIcons["Circle Collider2D"],
                [](auto &component)
                {
                    DrawVec2Control("Offset", component.Offset, 0.1f);
                    DrawFloatControl("Radius", component.Radius, 0.1f);
                    DrawFloatControl("Density", component.Density, 0.1f);
                    DrawFloatControl("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
                    DrawFloatControl("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
                    DrawFloatControl("Restitution Threshold", component.RestitutionThreshold, 0.1f);
                    DrawCheckboxControl("Is Trigger", component.IsTrigger);
                    component.Layer = std::clamp(component.Layer, 0, Physics2DLayerCount - 1);

                    int physicsLayerIndex = component.Layer;
                    Physics2DLayerSettings physicsLayerSettings;
                    if (Project::GetActive())
                        physicsLayerSettings = Project::GetConfig().Physics2DLayers;

                    std::vector<const char*> physicsLayerLabels;
                    physicsLayerLabels.reserve(Physics2DLayerCount);
                    for (int layerIndex = 0; layerIndex < Physics2DLayerCount; ++layerIndex)
                        physicsLayerLabels.push_back(
                            physicsLayerSettings.LayerNames[layerIndex].c_str());

                    DrawEnumComboControl(
                        "Layer", physicsLayerIndex, physicsLayerLabels.data(),
                        Physics2DLayerCount,
                        [&](int newIndex)
                        {
                            component.Layer = newIndex;
                        });
                });
        DrawComponent<SpriteAnimationComponent>(
                "Sprite Animation", entity, m_ComponentIcons["Sprite Animation"],
                [this, entity](auto &component)
                {
                    if (!entity.HasComponent<SpriteRendererComponent>())
                        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f),
                                           "需要 Sprite Renderer 组件才能显示动画。");

                    auto assetManager = Project::GetAssetManager();

                    std::string animationDisplayName = "None";
                    if (component.AnimationHandle != 0 && assetManager
                        && assetManager->IsAssetHandleValid(component.AnimationHandle))
                    {
                        const auto& metadata =
                            assetManager->GetAssetRegistry().at(component.AnimationHandle);
                        animationDisplayName = metadata.FilePath.filename().string();
                    }

                    DrawObjectReferenceField(
                        "Animation", animationDisplayName.c_str(), component.AnimationHandle != 0, nullptr,
                        [&]()
                        {
                            component.AnimationHandle = 0;
                        },
                        [&](const ImGuiPayload* payload)
                        {
                            const bool assigned = AssignAnimationAssetFromContentBrowserPayload(
                                    payload, component.AnimationHandle);
                            if (assigned && assetManager
                                && assetManager->IsAssetHandleValid(component.AnimationHandle))
                            {
                                Ref<SpriteAnimation> animation = std::static_pointer_cast<SpriteAnimation>(
                                        assetManager->GetAsset(component.AnimationHandle));
                                if (animation)
                                {
                                    if (component.CurrentAnimationName.empty()
                                        || !animation->HasAnimation(component.CurrentAnimationName))
                                    {
                                        if (const SpriteAnimationClip* primaryClip =
                                                animation->GetPrimaryClip())
                                            component.CurrentAnimationName = primaryClip->Name;
                                    }
                                    if (component.FrameRate <= 0.0f)
                                        component.FrameRate = animation->GetClipFrameRate(
                                                component.CurrentAnimationName);
                                }
                            }
                            return assigned;
                        },
                        [&]()
                        {
                            if (component.AnimationHandle == 0 || !assetManager
                                || !assetManager->IsAssetHandleValid(component.AnimationHandle))
                                return;

                            m_AnimationEditorRequest =
                                Project::GetAssetFileSystemPath(
                                    assetManager->GetAssetRegistry().at(component.AnimationHandle).FilePath);
                        });

                    const std::vector<std::string> animationNames = [&]() -> std::vector<std::string>
                    {
                        if (!assetManager || component.AnimationHandle == 0
                            || !assetManager->IsAssetHandleValid(component.AnimationHandle))
                            return {};
                        Ref<SpriteAnimation> animation = std::static_pointer_cast<SpriteAnimation>(
                                assetManager->GetAsset(component.AnimationHandle));
                        return animation ? animation->GetAnimationNames() : std::vector<std::string>{};
                    }();

                    if (!animationNames.empty())
                    {
                        int selectedAnimationIndex = 0;
                        for (size_t nameIndex = 0; nameIndex < animationNames.size(); ++nameIndex)
                        {
                            if (animationNames[nameIndex] == component.CurrentAnimationName)
                            {
                                selectedAnimationIndex = static_cast<int>(nameIndex);
                                break;
                            }
                        }

                        std::vector<const char*> animationNameLabels;
                        animationNameLabels.reserve(animationNames.size());
                        for (const std::string& animationName : animationNames)
                            animationNameLabels.push_back(animationName.c_str());

                        DrawEnumComboControl(
                                "Current Animation", selectedAnimationIndex,
                                animationNameLabels.data(),
                                static_cast<int>(animationNameLabels.size()),
                                [&](int newIndex)
                                {
                                    if (newIndex >= 0
                                        && newIndex < static_cast<int>(animationNames.size()))
                                    {
                                        component.CurrentAnimationName = animationNames[
                                                static_cast<size_t>(newIndex)];
                                        ResetSpriteAnimationPlayback(component);
                                    }
                                });
                    }
                    else
                    {
                        DrawReadOnlyTextControl("Current Animation",
                                                component.CurrentAnimationName.c_str());
                    }

                    DrawFloatControl("Frame Rate", component.FrameRate, 0.1f, 0.0f, 120.0f);
                    DrawCheckboxControl("Playing", component.Playing);
                    DrawCheckboxControl("Preview In Scene", component.PreviewInScene);

                    const int frameCount = [&]() -> int
                    {
                        if (!assetManager || component.AnimationHandle == 0
                            || !assetManager->IsAssetHandleValid(component.AnimationHandle))
                            return 0;
                        Ref<SpriteAnimation> animation = std::static_pointer_cast<SpriteAnimation>(
                                assetManager->GetAsset(component.AnimationHandle));
                        return animation
                            ? static_cast<int>(animation->GetFrameCount(
                                    component.CurrentAnimationName))
                            : 0;
                    }();

                    if (frameCount > 0)
                    {
                        int currentFrame = component.CurrentFrame;
                        DrawPropertyRow("Current Frame", [&]()
                        {
                            ImGui::PushItemWidth(-1.0f);
                            ImGui::SliderInt("##Value", &currentFrame, 0, frameCount - 1);
                            ImGui::PopItemWidth();
                        }, "编辑模式下 scrub 预览帧（需勾选 Preview In Scene）。");
                        component.CurrentFrame = currentFrame;
                    }
                    else
                    {
                        DrawReadOnlyTextControl("Current Frame", "0");
                    }

                    DrawHorizontalButtonPair("AnimationPlayback", [&]()
                    {
                        if (DrawTableFillButton("Reset Playback", "重置 Timer 与帧索引。"))
                            ResetSpriteAnimationPlayback(component);
                    }, [&]()
                    {
                        if (DrawTableFillButton("Stop", "停止播放。"))
                            component.Playing = false;
                    });
                });

        DrawComponent<TilemapComponent>(
                "Tilemap", entity, nullptr,
                [this](auto &component)
                {
                    auto assetManager = Project::GetAssetManager();

                    std::string tileMapDisplayName = "None";
                    if (component.TileMapHandle != 0 && assetManager
                        && assetManager->IsAssetHandleValid(component.TileMapHandle))
                    {
                        const auto &registry = assetManager->GetAssetRegistry();
                        const auto iterator = registry.find(component.TileMapHandle);
                        if (iterator != registry.end())
                            tileMapDisplayName = iterator->second.FilePath.filename().string();
                    }

                    DrawObjectReferenceField(
                        "Tile Map", tileMapDisplayName.c_str(), component.TileMapHandle != 0, nullptr,
                        [&]()
                        {
                            component.TileMapHandle = 0;
                        },
                        [&](const ImGuiPayload *payload)
                        {
                            if (!payload || !assetManager)
                                return false;

                            const wchar_t *path = (const wchar_t *)payload->Data;
                            std::filesystem::path assetPath(path);
                            if (assetPath.extension() != ".tilemap")
                                return false;

                            AssetHandle handle = assetManager->ImportAsset(assetPath);
                            if (handle == 0)
                                return false;

                            component.TileMapHandle = handle;
                            return true;
                        },
                        [&]()
                        {
                            if (component.TileMapHandle != 0)
                                m_TileMapEditorRequest = component.TileMapHandle;
                        });

                    if (component.TileMapHandle == 0)
                    {
                        DrawActionButtonRow("Tile Map", [&]()
                        {
                            if (ImGui::Button("Create New TileMap", ImVec2(-1.0f, 0.0f)))
                            {
                                if (!assetManager)
                                    return;

                                const auto assetDirectory = Project::GetAssetDirectory();

                                auto tileSetAsset = std::make_shared<TileSet>();
                                tileSetAsset->Handle = AssetHandle();

                                TileAtlasSource atlasSource;
                                atlasSource.TileSize = 16;
                                tileSetAsset->AddAtlasSource(atlasSource);

                                std::filesystem::path tileSetDirectory = assetDirectory / "tilesets";
                                std::filesystem::create_directories(tileSetDirectory);
                                std::filesystem::path tileSetPath = tileSetDirectory / "default.tileset";
                                int nameIndex = 0;
                                while (std::filesystem::exists(tileSetPath))
                                    tileSetPath = tileSetDirectory
                                        / ("default_" + std::to_string(++nameIndex) + ".tileset");

                                TileSetSerializer::Serialize(tileSetPath, tileSetAsset);
                                const auto tileSetRelativePath =
                                    std::filesystem::relative(tileSetPath, assetDirectory);
                                const AssetHandle tileSetHandle =
                                    assetManager->ImportAsset(tileSetRelativePath);
                                tileSetAsset->Handle = tileSetHandle;
                                TileSetSerializer::Serialize(tileSetPath, tileSetAsset);

                                auto mapAsset = std::make_shared<TileMapData>();
                                mapAsset->Handle = AssetHandle();
                                mapAsset->SetTileSetHandle(tileSetHandle);
                                mapAsset->SetCellSize(1.0f);

                                std::filesystem::path tileMapDirectory = assetDirectory / "tilemaps";
                                std::filesystem::create_directories(tileMapDirectory);
                                std::filesystem::path tileMapPath = tileMapDirectory / "new_tilemap.tilemap";
                                nameIndex = 0;
                                while (std::filesystem::exists(tileMapPath))
                                    tileMapPath = tileMapDirectory
                                        / ("new_tilemap_" + std::to_string(++nameIndex) + ".tilemap");

                                TileMapDataSerializer::Serialize(tileMapPath, mapAsset);
                                const auto tileMapRelativePath =
                                    std::filesystem::relative(tileMapPath, assetDirectory);
                                const AssetHandle tileMapHandle =
                                    assetManager->ImportAsset(tileMapRelativePath);
                                mapAsset->Handle = tileMapHandle;
                                TileMapDataSerializer::Serialize(tileMapPath, mapAsset);

                                assetManager->SerializeAssetRegistry();

                                component.TileMapHandle = tileMapHandle;
                                m_TileMapEditorRequest = tileMapHandle;
                            }
                        });
                    }
                    else if (assetManager && assetManager->IsAssetHandleValid(component.TileMapHandle))
                    {
                        DrawActionButtonRow("Tile Map", [&]()
                        {
                            if (ImGui::Button("Open TileMap Setup", ImVec2(-1.0f, 0.0f)))
                                m_TileMapEditorRequest = component.TileMapHandle;
                        });

                        ImGui::TextDisabled(
                            "选中实体显示 Scene 网格。在 TileMap Setup 右侧点选图块后再绘制；"
                            "Move Entity [H] 可移动实体。");
                    }
                });

        DrawComponent<TilemapCollider2DComponent>(
                "Tilemap Collider 2D", entity, nullptr,
                [this, entity](auto& component) mutable
                {
                    DrawCheckboxControl("Enabled", component.Enabled);
                    DrawCheckboxControl("Merge Adjacent Cells", component.MergeAdjacentCells);

                    if (!entity.HasComponent<TilemapComponent>())
                        return;

                    const TilemapComponent& tilemapComponent =
                            entity.GetComponent<TilemapComponent>();
                    if (tilemapComponent.TileMapHandle == 0 || !m_Context || !Project::GetActive())
                        return;

                    auto assetManager = Project::GetAssetManager();
                    if (!assetManager)
                        return;

                    Ref<TileMapData> mapData = std::static_pointer_cast<TileMapData>(
                            assetManager->GetAsset(tilemapComponent.TileMapHandle));
                    if (!mapData || mapData->GetTileSetHandle() == 0)
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
                                            "TileMap has no TileSet assigned.");
                        return;
                    }

                    Ref<TileSet> tileSet = std::static_pointer_cast<TileSet>(
                            assetManager->GetAsset(mapData->GetTileSetHandle()));
                    if (!tileSet)
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
                                            "TileSet asset could not be loaded.");
                        return;
                    }

                    const TilemapColliderBuildReport report =
                            TilemapColliderBuilder::AnalyzeCollidableCells(*mapData, *tileSet);

                    ImGui::Separator();
                    ImGui::Text("Painted cells: %u", report.paintedCellCount);
                    ImGui::Text("Collidable cells (Play): %u", report.collidableCellCount);
                    ImGui::Text("Unknown tile IDs: %u", report.orphanTileIdentifierCount);

                    if (report.collidableCellCount == 0)
                    {
                        ImGui::TextColored(
                                ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
                                "No collidable cells. Open TileMap Setup, enable Collidable on"
                                " tile types, Save TileSet, then Play.");
                    }
                    else if (report.orphanTileIdentifierCount > 0)
                    {
                        ImGui::TextColored(
                                ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
                                "Some painted cells use missing tile IDs. Re-slice or repaint.");
                    }
                });

        DrawComponent<ParticleEmitterComponent>(
                "Particle Emitter", entity, nullptr,
                [this](auto &component)
                {
                    auto assetManager = Project::GetAssetManager();
                    ImGui::PushID("ParticleEmitterHandle");
                    std::string label = "None (drag .particle)";
                    if (component.EmitterHandle != 0 && assetManager && assetManager->IsAssetHandleValid(component.EmitterHandle))
                        label = "Emitter: " + std::to_string((uint64_t)component.EmitterHandle);
                    ImGui::Button(label.c_str(), ImVec2(-1, 0.0f));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                        {
                            const wchar_t* path = (const wchar_t*)payload->Data;
                            std::filesystem::path assetPath(path);
                            if (assetPath.extension() == ".particle" && assetManager)
                            {
                                AssetHandle handle = assetManager->ImportAsset(assetPath);
                                if (handle != 0)
                                    component.EmitterHandle = handle;
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::PopID();
                    if (component.EmitterHandle == 0 && assetManager)
                    {
                        ImGui::Spacing();
                        if (ImGui::Button("Create New Particle Emitter", ImVec2(-1, 0)))
                        {
                            auto assetDir = Project::GetAssetDirectory();
                            std::filesystem::path dir = assetDir / "particles";
                            std::filesystem::create_directories(dir);
                            std::filesystem::path path = dir / "new_emitter.particle";
                            int idx = 0;
                            while (std::filesystem::exists(path))
                                path = dir / ("new_emitter_" + std::to_string(++idx) + ".particle");
                            auto emitterAsset = std::make_shared<ParticleEmitterAsset>();
                            emitterAsset->Handle = AssetHandle();
                            ParticleEmitterAssetSerializer::Serialize(path, emitterAsset);
                            auto relPath = std::filesystem::relative(path, assetDir);
                            AssetHandle handle = assetManager->ImportAsset(relPath);
                            if (handle != 0)
                            {
                                component.EmitterHandle = handle;
                                emitterAsset->Handle = handle;
                                ParticleEmitterAssetSerializer::Serialize(path, emitterAsset);
                                assetManager->SerializeAssetRegistry();
                            }
                        }
                    }
                    if (component.EmitterHandle != 0 && assetManager && assetManager->IsAssetHandleValid(component.EmitterHandle))
                    {
                        ImGui::Spacing();
                        if (ImGui::Button("Open in Particle Emitter Editor", ImVec2(-1, 0)))
                            m_ParticleEmitterEditorRequest = component.EmitterHandle;
                    }
                });

        //UI
        DrawComponent<UITransformComponent>("Rect Transform",
            entity, m_ComponentIcons["Rect Transform"],
            [this, entityIdentifier](auto &component)
            {
                UITransformComponent transformBeforeEdit = component;
                bool transformEditCaptured = false;

                auto beginTransformEdit = [&]()
                {
                    if (!transformEditCaptured)
                    {
                        transformBeforeEdit = component;
                        transformEditCaptured = true;
                    }
                };

                auto endTransformEdit = [&]()
                {
                    if (!transformEditCaptured || !m_CommandHistory)
                        return;

                    UITransformComponent transformAfterEdit = component;
                    if (transformBeforeEdit.Position != transformAfterEdit.Position
                        || transformBeforeEdit.Rotation != transformAfterEdit.Rotation
                        || transformBeforeEdit.Size != transformAfterEdit.Size)
                    {
                        m_CommandHistory->Execute(std::make_unique<ModifyUITransformCommand>(
                            m_Context, entityIdentifier, transformBeforeEdit, transformAfterEdit));
                    }
                    transformEditCaptured = false;
                };

                DrawVec3Control("Rect Position", component.Position, 0.0f,
                                beginTransformEdit, endTransformEdit);
                glm::vec3 rotation = glm::degrees(component.Rotation);
                DrawVec3Control("Rotation", rotation, 0.0f,
                                beginTransformEdit, endTransformEdit);
                component.Rotation = glm::radians(rotation);
                glm::vec3 sizeVector = glm::vec3(component.Size, 1.0f);
                DrawVec3Control("Size", sizeVector, 100.0f,
                                beginTransformEdit, endTransformEdit);
                component.Size = glm::vec2(sizeVector.x, sizeVector.y);
            });

        DrawComponent<UIImageComponent>("Image", entity, m_ComponentIcons["Image"],
        [](auto &component)
        {
                    // 1. 颜色控制 (直接复用你的 DrawColorControl)
                    DrawColorControl("Color", component.Color);

                    DrawTextureAssignControl("Texture", component.Texture, component.TextureHandle);
        });
        DrawComponent<UITextComponent>("Text", entity, m_ComponentIcons["Text"],
                                       [](auto &component)
                                       {
                                           // 1. 文本内容编辑 (支持多行)
                                           ImGui::Text("Content");
                                           ImGui::PushID("Content");
                                           char buffer[1024];
                                           memset(buffer, 0, sizeof(buffer));
                                           snprintf(buffer, sizeof(buffer), "%s", component.TextString.c_str());
                                           if (ImGui::InputTextMultiline("##TextContent", buffer, sizeof(buffer),
                                                                         ImVec2(-1, ImGui::GetFontSize() * 3)))
                                           {
                                               component.TextString = std::string(buffer);
                                           }
                                           ImGui::PopID();

                                           // 2. 颜色
                                           DrawColorControl("Text Color", component.Color);

                                           // 3. 排版参数
                                           DrawFloatControl("Kerning", component.Kerning, 0.01f);
                                           DrawFloatControl("Line Spacing", component.LineSpacing, 0.01f);

                                           // 4. 字体资产显示 (暂时只读，或者你可以像图片一样实现拖拽)
                                           ImGui::Text("Font: %s", component.FontAsset ? "Default Font" : "None");
                                           if (!component.FontAsset)
                                           {
                                               if (ImGui::Button("Attach Default Font"))
                                                   component.FontAsset = Font::GetDefault();
                                           }
                                       });
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
