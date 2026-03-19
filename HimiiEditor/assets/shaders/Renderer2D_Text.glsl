#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout(location = 0) out VertexOutput Output;
layout(location = 2) out flat int v_EntityID;

void main()
{
    Output.Color = a_Color;
    Output.TexCoord = a_TexCoord;
    v_EntityID = a_EntityID;
    
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2; // 用于鼠标拾取实体ID

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) in VertexOutput Input;
layout (location = 2) in flat int v_EntityID;

layout (set=1,binding = 0) uniform sampler2D u_FontAtlas;

float screenPxRange() {
    const float pxRange = 2.0; 
    vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlas, 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(Input.TexCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

// 求三个通道的中位数 (MSDF 的核心算法)
float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3 msd = texture(u_FontAtlas, Input.TexCoord).rgb;
    
    // 2. 解码距离场
    float sd = median(msd.r, msd.g, msd.b);
    
    // 3. 计算屏幕空间内的精确边缘 (抗锯齿)
    float screenPxDistance = screenPxRange() * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    
    // 如果完全透明，直接丢弃像素，节省性能
    if (opacity < 0.01)
        discard;

    // 4. 混合字体颜色
    vec4 bgColor = vec4(0.0); // 背景是透明的
    color = mix(bgColor, Input.Color, opacity);
    
    if (color.a < 0.01)
        discard;

    // 5. 写入 EntityID 供鼠标拾取使用
    color2 = v_EntityID;
}