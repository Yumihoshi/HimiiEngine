#include "panel/ComponentInspector/ComponentInspectorDrawContext.h"
#include "panel/ComponentInspector/ComponentInspectorHeader.h"
#include "panel/ComponentInspector/ComponentInspectorRegistry.h"
#include "InspectorControls.h"

#include "Himii/Asset/AssetManager.h"
#include "Himii/Audio/AudioEngine.h"
#include "Himii/Audio/SoundAsset.h"
#include "Himii/Audio/SoundPlayerUtility.h"
#include "Himii/Project/Project.h"
#include "Himii/Scene/Components.h"

#include <filesystem>
#include <imgui.h>

namespace Himii
{
    static bool IsSoundFileExtension(const std::filesystem::path& path)
    {
        const auto extension = path.extension().string();
        return extension == ".wav" || extension == ".ogg" || extension == ".mp3"
               || extension == ".WAV" || extension == ".OGG" || extension == ".MP3";
    }

    static void DrawSoundPlayerComponentInspectorUI(ComponentInspectorDrawContext& drawContext)
    {
        if (!drawContext.entity.HasComponent<SoundPlayerComponent>())
            return;

        auto& component = drawContext.entity.GetComponent<SoundPlayerComponent>();

        DrawComponentInspectorHeaderUI(
            drawContext, "SoundPlayerComponent", "Sound Player", nullptr,
            [&]()
            {
                auto assetManager = Project::GetAssetManager();
                ImGui::PushID("SoundPlayerSoundHandle");
                std::string label = "None (drag .wav/.ogg/.mp3)";
                if (component.SoundHandle != 0 && assetManager
                    && assetManager->IsAssetHandleValid(component.SoundHandle))
                {
                    const auto& registry = assetManager->GetAssetRegistry();
                    auto iterator = registry.find(component.SoundHandle);
                    if (iterator != registry.end())
                        label = iterator->second.FilePath.filename().string();
                    else
                        label = "Sound: " + std::to_string((uint64_t)component.SoundHandle);
                }
                ImGui::Button(label.c_str(), ImVec2(-1.0f, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* pathWide = (const wchar_t*)payload->Data;
                        std::filesystem::path assetPath(pathWide);
                        if (IsSoundFileExtension(assetPath) && assetManager)
                        {
                            std::filesystem::path relativePath =
                                    std::filesystem::relative(assetPath, Project::GetAssetDirectory());
                            AssetHandle handle = assetManager->ImportAsset(relativePath);
                            if (handle)
                            {
                                component.SoundHandle = handle;
                                SoundPlayerUtility::ResolveSoundAsset(component);
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();

                float volume = component.Volume;
                DrawFloatControl("Volume", volume, 0.01f, 0.0f, 1.0f);
                if (volume != component.Volume)
                {
                    component.Volume = volume;
                    SoundPlayerUtility::ApplyVolume(component);
                }

                bool mute = component.Mute;
                DrawPropertyRow("Mute", [&]() { ImGui::Checkbox("##Mute", &mute); });
                if (mute != component.Mute)
                {
                    component.Mute = mute;
                    SoundPlayerUtility::ApplyVolume(component);
                }

                DrawPropertyRow("Loop", [&]() { ImGui::Checkbox("##Loop", &component.Loop); });
                DrawPropertyRow("Play On Start",
                                [&]() { ImGui::Checkbox("##PlayOnStart", &component.PlayOnStart); });

                if (ImGui::Button("Preview", ImVec2(120.0f, 0.0f)))
                {
                    SoundPlayerUtility::ResolveSoundAsset(component);
                    if (component.Sound)
                        AudioEngine::Preview(component.Sound, component.EvaluateEffectiveVolume());
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop Preview", ImVec2(120.0f, 0.0f)))
                    AudioEngine::StopPreview();
            },
            [&]()
            {
                SoundPlayerUtility::Stop(component);
                drawContext.entity.RemoveComponent<SoundPlayerComponent>();
            });
    }

    struct SoundPlayerComponentInspectorRegistrar
    {
        SoundPlayerComponentInspectorRegistrar()
        {
            ComponentInspectorRegistry::Get().RegisterComponentInspector<SoundPlayerComponent>(
                "SoundPlayerComponent", "Sound Player", "Audio", 260,
                &DrawSoundPlayerComponentInspectorUI);
        }
    };

    static SoundPlayerComponentInspectorRegistrar s_SoundPlayerComponentInspectorRegistrar;
}
