#pragma once

#include "Himii/Asset/Asset.h"
#include "Himii/Renderer/Texture.h"

namespace Himii
{
    void DrawTextureAssignControl(const char* label, Ref<Texture2D>& texture, AssetHandle& textureHandle);
}
