#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

// 与 Renderer2D_Quad 保持相同的 location 占用与结构体用法，
// 避免 Vulkan SPIR-V → SPIRV-Cross → OpenGL SPIR-V 链路下的阶段接口失配。
struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
    float AtlasPixelRange;
};

layout(location = 0) out VertexOutput Output;
layout(location = 3) out flat float v_TexIndex;
layout(location = 4) out flat int v_EntityID;

void main()
{
    Output.Color = a_Color;
    Output.TexCoord = a_TexCoord;
    Output.AtlasPixelRange = 2.0;
    v_TexIndex = a_TexIndex;
    v_EntityID = a_EntityID;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
    float AtlasPixelRange;
};

layout(location = 0) in VertexOutput Input;
layout(location = 3) in flat float v_TexIndex;
layout(location = 4) in flat int v_EntityID;

layout(binding = 0) uniform sampler2D u_FontAtlases[32];

void main()
{
    // 关键：必须让结构体三个成员都参与与 Quad 相同形态的采样表达式，
    // 否则 SPIRV-Cross 二次编译后会出现 “_xx._m0 not declared as an output” 链接错误。
    // 这里用 (Input.Color * 0 + 1) 得到单位向量，再 *= texture，既保住接口，又拿到纯净 MSDF 采样。
    vec4 atlasSample = (Input.Color * 0.0) + vec4(1.0);
    atlasSample *= max(Input.AtlasPixelRange, 1e-4) / max(Input.AtlasPixelRange, 1e-4);
    switch (int(v_TexIndex))
    {
        case  0: atlasSample *= texture(u_FontAtlases[ 0], Input.TexCoord); break;
        case  1: atlasSample *= texture(u_FontAtlases[ 1], Input.TexCoord); break;
        case  2: atlasSample *= texture(u_FontAtlases[ 2], Input.TexCoord); break;
        case  3: atlasSample *= texture(u_FontAtlases[ 3], Input.TexCoord); break;
        case  4: atlasSample *= texture(u_FontAtlases[ 4], Input.TexCoord); break;
        case  5: atlasSample *= texture(u_FontAtlases[ 5], Input.TexCoord); break;
        case  6: atlasSample *= texture(u_FontAtlases[ 6], Input.TexCoord); break;
        case  7: atlasSample *= texture(u_FontAtlases[ 7], Input.TexCoord); break;
        case  8: atlasSample *= texture(u_FontAtlases[ 8], Input.TexCoord); break;
        case  9: atlasSample *= texture(u_FontAtlases[ 9], Input.TexCoord); break;
        case 10: atlasSample *= texture(u_FontAtlases[10], Input.TexCoord); break;
        case 11: atlasSample *= texture(u_FontAtlases[11], Input.TexCoord); break;
        case 12: atlasSample *= texture(u_FontAtlases[12], Input.TexCoord); break;
        case 13: atlasSample *= texture(u_FontAtlases[13], Input.TexCoord); break;
        case 14: atlasSample *= texture(u_FontAtlases[14], Input.TexCoord); break;
        case 15: atlasSample *= texture(u_FontAtlases[15], Input.TexCoord); break;
        case 16: atlasSample *= texture(u_FontAtlases[16], Input.TexCoord); break;
        case 17: atlasSample *= texture(u_FontAtlases[17], Input.TexCoord); break;
        case 18: atlasSample *= texture(u_FontAtlases[18], Input.TexCoord); break;
        case 19: atlasSample *= texture(u_FontAtlases[19], Input.TexCoord); break;
        case 20: atlasSample *= texture(u_FontAtlases[20], Input.TexCoord); break;
        case 21: atlasSample *= texture(u_FontAtlases[21], Input.TexCoord); break;
        case 22: atlasSample *= texture(u_FontAtlases[22], Input.TexCoord); break;
        case 23: atlasSample *= texture(u_FontAtlases[23], Input.TexCoord); break;
        case 24: atlasSample *= texture(u_FontAtlases[24], Input.TexCoord); break;
        case 25: atlasSample *= texture(u_FontAtlases[25], Input.TexCoord); break;
        case 26: atlasSample *= texture(u_FontAtlases[26], Input.TexCoord); break;
        case 27: atlasSample *= texture(u_FontAtlases[27], Input.TexCoord); break;
        case 28: atlasSample *= texture(u_FontAtlases[28], Input.TexCoord); break;
        case 29: atlasSample *= texture(u_FontAtlases[29], Input.TexCoord); break;
        case 30: atlasSample *= texture(u_FontAtlases[30], Input.TexCoord); break;
        case 31: atlasSample *= texture(u_FontAtlases[31], Input.TexCoord); break;
    }

    float signedDistance = max(
            min(atlasSample.r, atlasSample.g),
            min(max(atlasSample.r, atlasSample.g), atlasSample.b));

    vec2 unitRange = vec2(Input.AtlasPixelRange) / vec2(textureSize(u_FontAtlases[0], 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(Input.TexCoord);
    float screenPixelRange = max(0.5 * dot(unitRange, screenTexSize), 1.0);
    float coverage = clamp(screenPixelRange * (signedDistance - 0.5) + 0.5, 0.0, 1.0);

    if (coverage < 0.01)
        discard;

    // Straight Alpha：RGB 保持字体颜色，仅 Alpha 乘覆盖率。
    color = vec4(Input.Color.rgb, Input.Color.a * coverage);
    if (color.a < 0.01)
        discard;

    color2 = v_EntityID;
}
