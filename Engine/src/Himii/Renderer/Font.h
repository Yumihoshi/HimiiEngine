#pragma once
#include <filesystem>
#include "Himii/Core/Core.h"
#include "Himii/Renderer/Texture.h"

#include "freetype/freetype.h"
// 消除 Windows 下的一些宏定义冲突
#undef INFINITE
#include "msdf-atlas-gen/FontGeometry.h"
#include "msdf-atlas-gen/GlyphGeometry.h"
#include "msdf-atlas-gen/msdf-atlas-gen.h"

#include "msdfgen/msdfgen.h"
#include "msdfgen/msdfgen-ext.h"

namespace Himii
{
    struct MSDFData {
        std::vector<msdf_atlas::GlyphGeometry> Glyphs;
        msdf_atlas::FontGeometry FontGeometry;
    };

    class Font {
    public:
        Font(const std::filesystem::path &filepath);
        ~Font();

        Ref<Texture2D> GetAtlasTexture() const
        {
            return m_AtlasTexture;
        }
        const MSDFData *GetMSDFData() const
        {
            return m_Data;
        }

        static Ref<Font> GetDefault();
        static void InitDefault(const std::filesystem::path &path);

        const std::filesystem::path &GetFilePath() const
        {
            return m_FilePath;
        }

    private:
        MSDFData *m_Data;
        Ref<Texture2D> m_AtlasTexture;
        static Ref<Font> s_DefaultFont;

        std::filesystem::path m_FilePath;
    };
} // namespace Himii
