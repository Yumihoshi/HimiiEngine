#include "Font.h"
#include "Hepch.h"
#include "Himii/Core/FileSystem.h"

using namespace msdf_atlas;

namespace Himii
{
    Ref<Font> Font::s_DefaultFont = nullptr;
    void Font::InitDefault(const std::filesystem::path &path)
    {
        s_DefaultFont = CreateRef<Font>(path);
    }
    Ref<Font> Font::GetDefault()
    {
        return s_DefaultFont;
    }

    Font::Font(const std::filesystem::path &filepath)
    {
        m_Data = new MSDFData();
        
        // 1. 初始化 Freetype
        msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype();
        HIMII_CORE_ASSERT(ft, "Failed to initialize FreeType!");

        // 2. 加载字体文件
        const std::filesystem::path resolvedFontPath = FileSystem::MaterializeLooseFile(filepath.string());
        msdfgen::FontHandle *font = msdfgen::loadFont(ft, resolvedFontPath.string().c_str());

        // 3. 设定要渲染的字符集 (这里先以 ASCII 为例，支持 0x0020 到 0x00FF)
        msdf_atlas::Charset charset;
        static const uint32_t charsetRanges[] = {0x0020, 0x00FF};
        for (int i = 0; i < 2; i += 2)
            for (uint32_t c = charsetRanges[i]; c <= charsetRanges[i + 1]; c++)
                charset.add(c);

        m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
        m_Data->FontGeometry.loadCharset(font, 1.0, charset);

        // 4. 配置 Atlas 打包器
        msdf_atlas::TightAtlasPacker packer;
        packer.setDimensionsConstraint(msdf_atlas::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE); // 强制方形纹理
        packer.setMinimumScale(40.0);
        packer.setPixelRange(2.0); // 距离场影响范围，极其关键的参数
        packer.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());

        int width = 0, height = 0;
        packer.getDimensions(width, height);

        // 5. 多线程生成 MSDF 位图 (RGB三个通道)
        using GeneratorType = msdf_atlas::ImmediateAtlasGenerator<float, 4, &msdf_atlas::mtsdfGenerator,
                                                                  msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4>>;

        GeneratorType generator(width, height);
        msdf_atlas::GeneratorAttributes attributes;
        generator.setAttributes(attributes);
        generator.setThreadCount(8); // 开启 8 线程加速烘焙
        generator.generate(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());

        msdfgen::BitmapConstRef<msdf_atlas::byte, 4> bitmap = generator.atlasStorage();

        // 6. 将 MSDF 数据转入 Hazel/Himii 的 Texture2D 中
        // 注意：如果你现在的 Texture2D::Create 还不支持传参数，请确保它有一个支持 (width, height, format, data) 的重载
        m_AtlasTexture = Texture2D::Create(width, height);

        // 假设你有 SetData 方法将 CPU 内存上传至 GPU
        // 格式应当是 RGB8 (3 channels)
        m_AtlasTexture->SetData((void *)bitmap.pixels, bitmap.width * bitmap.height * 4);

        // 清理
        msdfgen::destroyFont(font);
        msdfgen::deinitializeFreetype(ft);
    }

    Font::~Font()
    {
        delete m_Data;
    }
} // namespace Himii
