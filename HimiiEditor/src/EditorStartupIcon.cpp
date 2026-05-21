#include "EditorStartupIcon.h"

#include "Himii/Renderer/OrthographicCamera.h"
#include "Himii/Renderer/RenderCommand.h"
#include "Himii/Renderer/Renderer2D.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>

#include "stb_image.h"

#ifdef HIMII_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace Himii
{
    namespace
    {
        constexpr uint32_t StartupWindowPaddingPixels = 8;
        constexpr uint8_t AlphaThreshold = 8;

#ifdef HIMII_PLATFORM_WINDOWS
        static bool LoadIconFileWindows(const char *file_path, StartupIconImage &out_image)
        {
            HICON icon_handle = static_cast<HICON>(LoadImageA(
                nullptr, file_path, IMAGE_ICON, 256, 256, LR_LOADFROMFILE | LR_DEFAULTCOLOR));
            if (!icon_handle)
                return false;

            ICONINFO icon_info = {};
            if (!GetIconInfo(icon_handle, &icon_info))
            {
                DestroyIcon(icon_handle);
                return false;
            }

            HBITMAP color_bitmap = icon_info.hbmColor ? icon_info.hbmColor : icon_info.hbmMask;
            if (!color_bitmap)
            {
                if (icon_info.hbmColor)
                    DeleteObject(icon_info.hbmColor);
                if (icon_info.hbmMask)
                    DeleteObject(icon_info.hbmMask);
                DestroyIcon(icon_handle);
                return false;
            }

            BITMAP bitmap_info = {};
            GetObject(color_bitmap, sizeof(BITMAP), &bitmap_info);

            const int bitmap_width = bitmap_info.bmWidth;
            const int bitmap_height = bitmap_info.bmHeight;
            if (bitmap_width <= 0 || bitmap_height <= 0)
            {
                if (icon_info.hbmColor)
                    DeleteObject(icon_info.hbmColor);
                if (icon_info.hbmMask)
                    DeleteObject(icon_info.hbmMask);
                DestroyIcon(icon_handle);
                return false;
            }

            BITMAPINFO header = {};
            header.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            header.bmiHeader.biWidth = bitmap_width;
            header.bmiHeader.biHeight = -bitmap_height;
            header.bmiHeader.biPlanes = 1;
            header.bmiHeader.biBitCount = 32;
            header.bmiHeader.biCompression = BI_RGB;

            out_image.Width = (uint32_t)bitmap_width;
            out_image.Height = (uint32_t)bitmap_height;
            out_image.PixelsRgba.resize((size_t)bitmap_width * (size_t)bitmap_height * 4);

            HDC device_context = GetDC(nullptr);
            const int copied_scanlines = GetDIBits(device_context, color_bitmap, 0, bitmap_height,
                                                    out_image.PixelsRgba.data(), &header, DIB_RGB_COLORS);
            ReleaseDC(nullptr, device_context);

            if (icon_info.hbmColor)
                DeleteObject(icon_info.hbmColor);
            if (icon_info.hbmMask)
                DeleteObject(icon_info.hbmMask);
            DestroyIcon(icon_handle);

            if (copied_scanlines == 0)
            {
                out_image = {};
                return false;
            }

            for (size_t index = 0; index < out_image.PixelsRgba.size(); index += 4)
            {
                const uint8_t blue = out_image.PixelsRgba[index + 0];
                const uint8_t green = out_image.PixelsRgba[index + 1];
                const uint8_t red = out_image.PixelsRgba[index + 2];
                const uint8_t alpha = out_image.PixelsRgba[index + 3];
                out_image.PixelsRgba[index + 0] = red;
                out_image.PixelsRgba[index + 1] = green;
                out_image.PixelsRgba[index + 2] = blue;
                out_image.PixelsRgba[index + 3] = alpha;
            }

            return true;
        }
#endif

        static bool LoadImageFileStb(const char *file_path, StartupIconImage &out_image)
        {
            int width = 0;
            int height = 0;
            int channels = 0;
            stbi_set_flip_vertically_on_load(1);
            stbi_uc *image_data = stbi_load(file_path, &width, &height, &channels, 4);
            if (!image_data || width <= 0 || height <= 0)
                return false;

            out_image.Width = (uint32_t)width;
            out_image.Height = (uint32_t)height;
            out_image.PixelsRgba.assign(image_data, image_data + (size_t)width * (size_t)height * 4);
            stbi_image_free(image_data);
            return true;
        }
    }

    namespace EditorStartupIcon
    {
        bool LoadEngineIcon(StartupIconImage &out_image)
        {
            const std::array<const char *, 2> candidate_paths = {
                "resources/icons/HimiiEngine.ico",
                "resources/icons/HimiiEngine.png",
            };

            for (const char *candidate_path : candidate_paths)
            {
                if (!std::filesystem::exists(candidate_path))
                    continue;

#ifdef HIMII_PLATFORM_WINDOWS
                const size_t path_length = std::strlen(candidate_path);
                const bool is_icon_file = path_length >= 4 &&
                                          std::strcmp(candidate_path + path_length - 4, ".ico") == 0;
                if (is_icon_file)
                {
                    if (LoadIconFileWindows(candidate_path, out_image))
                        return true;
                    continue;
                }
#endif
                if (LoadImageFileStb(candidate_path, out_image))
                    return true;
            }

            return false;
        }

        void CropTransparentPixels(StartupIconImage &image, uint8_t alpha_threshold)
        {
            if (image.Width == 0 || image.Height == 0 || image.PixelsRgba.empty())
                return;

            uint32_t minimum_x = image.Width;
            uint32_t minimum_y = image.Height;
            uint32_t maximum_x = 0;
            uint32_t maximum_y = 0;
            bool found_opaque_pixel = false;

            for (uint32_t y = 0; y < image.Height; ++y)
            {
                for (uint32_t x = 0; x < image.Width; ++x)
                {
                    const size_t pixel_index = ((size_t)y * image.Width + x) * 4;
                    const uint8_t alpha = image.PixelsRgba[pixel_index + 3];
                    if (alpha <= alpha_threshold)
                        continue;

                    found_opaque_pixel = true;
                    minimum_x = std::min(minimum_x, x);
                    minimum_y = std::min(minimum_y, y);
                    maximum_x = std::max(maximum_x, x);
                    maximum_y = std::max(maximum_y, y);
                }
            }

            if (!found_opaque_pixel)
                return;

            const uint32_t cropped_width = maximum_x - minimum_x + 1;
            const uint32_t cropped_height = maximum_y - minimum_y + 1;
            std::vector<uint8_t> cropped_pixels((size_t)cropped_width * (size_t)cropped_height * 4);

            for (uint32_t y = 0; y < cropped_height; ++y)
            {
                for (uint32_t x = 0; x < cropped_width; ++x)
                {
                    const size_t source_index =
                        ((size_t)(minimum_y + y) * image.Width + (minimum_x + x)) * 4;
                    const size_t destination_index = ((size_t)y * cropped_width + x) * 4;
                    cropped_pixels[destination_index + 0] = image.PixelsRgba[source_index + 0];
                    cropped_pixels[destination_index + 1] = image.PixelsRgba[source_index + 1];
                    cropped_pixels[destination_index + 2] = image.PixelsRgba[source_index + 2];
                    cropped_pixels[destination_index + 3] = image.PixelsRgba[source_index + 3];
                }
            }

            image.Width = cropped_width;
            image.Height = cropped_height;
            image.PixelsRgba = std::move(cropped_pixels);
        }

        Ref<Texture2D> CreateTexture(const StartupIconImage &image)
        {
            if (image.Width == 0 || image.Height == 0 || image.PixelsRgba.empty())
                return nullptr;

            Ref<Texture2D> texture = Texture2D::Create(image.Width, image.Height);
            texture->SetData((void *)image.PixelsRgba.data(),
                             image.Width * image.Height * 4);
            return texture;
        }

        void DrawStartupFrame(const Ref<Texture2D> &icon_texture, uint32_t window_width,
                              uint32_t window_height)
        {
            RenderCommand::SetClearColor(glm::vec4{0.0f, 0.0f, 0.0f, 0.0f});
            RenderCommand::Clear();

            if (!icon_texture)
                return;

            const float half_width = (float)window_width * 0.5f;
            const float half_height = (float)window_height * 0.5f;
            OrthographicCamera camera(-half_width, half_width, -half_height, half_height);

            Renderer2D::BeginScene(camera);
            const glm::vec2 quad_size = {(float)icon_texture->GetWidth(), (float)icon_texture->GetHeight()};
            const glm::vec2 centered_position = {-quad_size.x * 0.5f, -quad_size.y * 0.5f};
            Renderer2D::DrawQuad(centered_position, quad_size, icon_texture, 1.0f, glm::vec4{1.0f});
            Renderer2D::EndScene();
            Renderer2D::Flush();
        }

        uint32_t GetStartupWindowWidth(const StartupIconImage &image)
        {
            return image.Width + StartupWindowPaddingPixels * 2;
        }

        uint32_t GetStartupWindowHeight(const StartupIconImage &image)
        {
            return image.Height + StartupWindowPaddingPixels * 2;
        }
    }
}
