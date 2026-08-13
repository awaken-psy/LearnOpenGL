// SSAO Geometry Pass 片段着色器 — MRT 写 3 张 G-Buffer(view space)
//
// 与 8.1.g_buffer.fs 的差异:
//   - 只输出 vec3 gAlbedo(没 alpha 通道,因为这个 demo 不用 specular 纹理)
//   - gAlbedo 硬编码 0.95(统一灰白色),不采样模型纹理 —— SSAO 只关心遮蔽,颜色不重要
#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

void main()
{
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // 漫反射色硬编码 0.95(均匀灰白),简化 demo,聚焦 SSAO 算法本身。
    // and the diffuse per-fragment color
    gAlbedo.rgb = vec3(0.95);
}