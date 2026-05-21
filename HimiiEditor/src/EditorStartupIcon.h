#pragma once

#include "Himii/Renderer/Texture.h"

#include <cstdint>
#include <vector>

namespace Himii
{
    struct StartupIconImage
    {
        uint32_t Width = 0;
        uint32_t Height = 0;
        std::vector<uint8_t> PixelsRgba;
    };

    namespace EditorStartupIcon
    {
        bool LoadEngineIcon(StartupIconImage &out_image);

        void CropTransparentPixels(StartupIconImage &image, uint8_t alpha_threshold = 8);

        Ref<Texture2D> CreateTexture(const StartupIconImage &image);

        void DrawStartupFrame(const Ref<Texture2D> &icon_texture, uint32_t window_width, uint32_t window_height);

        uint32_t GetStartupWindowWidth(const StartupIconImage &image);
        uint32_t GetStartupWindowHeight(const StartupIconImage &image);
    }
}
