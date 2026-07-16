#include "ComponentInspectorUtils.h"

#include "InspectorControls.h"

#include <filesystem>

namespace Himii
{
    void DrawTextureAssignControl(const char* label, Ref<Texture2D>& texture, AssetHandle& textureHandle)
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
}
