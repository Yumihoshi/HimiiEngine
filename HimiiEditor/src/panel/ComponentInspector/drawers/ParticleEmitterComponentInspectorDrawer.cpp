#include "panel/ComponentInspector/ComponentInspectorDrawContext.h"
#include "panel/ComponentInspector/ComponentInspectorHeader.h"
#include "panel/ComponentInspector/ComponentInspectorRegistry.h"

#include "Himii/Asset/AssetManager.h"
#include "Himii/Asset/AssetSerializer.h"
#include "Himii/Project/Project.h"
#include "Himii/Scene/Components.h"
#include "Himii/Scene/ParticleEmitterAsset.h"

#include <filesystem>
#include <imgui.h>

namespace Himii
{
    static void DrawParticleEmitterComponentInspectorUI(ComponentInspectorDrawContext& drawContext)
    {
        if (!drawContext.entity.HasComponent<ParticleEmitterComponent>())
            return;

        auto& component = drawContext.entity.GetComponent<ParticleEmitterComponent>();

        DrawComponentInspectorHeaderUI(
            drawContext, "ParticleEmitterComponent", "Particle Emitter", nullptr,
            [&]()
            {
                auto assetManager = Project::GetAssetManager();
                ImGui::PushID("ParticleEmitterHandle");
                std::string label = "None (drag .particle)";
                if (component.EmitterHandle != 0 && assetManager
                    && assetManager->IsAssetHandleValid(component.EmitterHandle))
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
                        auto assetDirectory = Project::GetAssetDirectory();
                        std::filesystem::path directory = assetDirectory / "particles";
                        std::filesystem::create_directories(directory);
                        std::filesystem::path path = directory / "new_emitter.particle";
                        int nameIndex = 0;
                        while (std::filesystem::exists(path))
                            path = directory / ("new_emitter_" + std::to_string(++nameIndex) + ".particle");

                        auto emitterAsset = std::make_shared<ParticleEmitterAsset>();
                        emitterAsset->Handle = AssetHandle();
                        ParticleEmitterAssetSerializer::Serialize(path, emitterAsset);
                        auto relativePath = std::filesystem::relative(path, assetDirectory);
                        AssetHandle handle = assetManager->ImportAsset(relativePath);
                        if (handle != 0)
                        {
                            component.EmitterHandle = handle;
                            emitterAsset->Handle = handle;
                            ParticleEmitterAssetSerializer::Serialize(path, emitterAsset);
                            assetManager->SerializeAssetRegistry();
                        }
                    }
                }

                if (component.EmitterHandle != 0 && assetManager
                    && assetManager->IsAssetHandleValid(component.EmitterHandle))
                {
                    ImGui::Spacing();
                    if (ImGui::Button("Open in Particle Emitter Editor", ImVec2(-1, 0))
                        && drawContext.requestParticleEmitterEditor)
                        drawContext.requestParticleEmitterEditor(component.EmitterHandle);
                }
            },
            [&]() { drawContext.entity.RemoveComponent<ParticleEmitterComponent>(); });
    }

    struct ParticleEmitterComponentInspectorRegistrar
    {
        ParticleEmitterComponentInspectorRegistrar()
        {
            ComponentInspectorRegistry::Get().RegisterComponentInspector<ParticleEmitterComponent>(
                "ParticleEmitterComponent", "Particle Emitter", "", 120,
                &DrawParticleEmitterComponentInspectorUI);
        }
    };

    static ParticleEmitterComponentInspectorRegistrar s_ParticleEmitterComponentInspectorRegistrar;
}
