#include "Hepch.h"
#include "Himii/Core/Log.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <array>

namespace Himii
{
    namespace Utils
    {

        static GLenum HazelImageFormatToGLDataFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::RGB8:
                    return GL_RGB;
                case ImageFormat::RGBA8:
                    return GL_RGBA;
            }

            HIMII_CORE_ASSERT(false);
            return 0;
        }

        static GLenum HazelImageFormatToGLInternalFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::RGB8:
                    return GL_RGB8;
                case ImageFormat::RGBA8:
                    return GL_RGBA8;
            }

            HIMII_CORE_ASSERT(false);
            return 0;
        }

    } // namespace Utils
    OpenGLTexture::OpenGLTexture(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
    {
        HIMII_PROFILE_FUNCTION();

         m_InternalFormat = GL_RGBA8;
         m_DataFormat = GL_RGBA;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        m_IsLoaded = true;
    }
    OpenGLTexture::OpenGLTexture(const std::string &path) : m_Path(path)
    {
        HIMII_PROFILE_FUNCTION();

        int width = 0;
        int height = 0;
        int source_channels = 0;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc *data = nullptr;
        {
            HIMII_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&)");
            data = stbi_load(path.c_str(), &width, &height, &source_channels, 4);
        }
        HIMII_CORE_ASSERT(data, "Failed to load image!");
        m_Width = (uint32_t)width;
        m_Height = (uint32_t)height;

        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        stbi_image_free(data);
        m_IsLoaded = true;
    }
    OpenGLTexture::~OpenGLTexture()
    {
        HIMII_PROFILE_FUNCTION();

        glDeleteTextures(1, &m_RendererID);
    }
    void OpenGLTexture::SetData(void *data, uint32_t size)
    {
        HIMII_PROFILE_FUNCTION();

        uint32_t bpp= m_DataFormat == GL_RGBA ? 4:3;
        HIMII_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
        glTextureSubImage2D(m_RendererID, 0, 0, 0,m_Width,m_Height,m_DataFormat,GL_UNSIGNED_BYTE,data);
    }
    void OpenGLTexture::Bind(uint32_t slot) const
    {
        HIMII_PROFILE_FUNCTION();

        glBindTextureUnit(slot, m_RendererID);
    }
} // namespace Himii
