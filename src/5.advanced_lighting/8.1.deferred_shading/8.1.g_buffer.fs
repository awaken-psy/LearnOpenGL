// Geometry Pass 片段着色器 — ⭐【MRT 多渲染目标】一次性把几何信息写到 3 张 G-Buffer 纹理
//
// 关键就是 layout(location = X):指定这个输出变量写到【哪个颜色附件】。
//   location=0 → GL_COLOR_ATTACHMENT0(gPosition,对应 cpp 里挂的第 1 张纹理)
//   location=1 → GL_COLOR_ATTACHMENT1(gNormal, 对应第 2 张)
//   location=2 → GL_COLOR_ATTACHMENT2(gAlbedoSpec,对应第 3 张)
// 一次 draw 调用,3 个 out 变量同时写入 3 张纹理 —— 这就是 MRT 的威力。
#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    // 写入第 1 张:fragment 世界空间位置
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // 写入第 2 张:归一化法线(插值后长度会变,必须重新归一化)
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // 写入第 3 张的 rgb:漫反射颜色(从模型漫反射纹理采样)
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    // 写入第 3 张的 a:高光强度(从模型 specular 纹理采样的 r 通道,标量)
    //   ⚠ 巧妙复用:diffuse 占 rgb、specular 占 a,塞进同一张 RGBA 纹理,省一张。
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
}