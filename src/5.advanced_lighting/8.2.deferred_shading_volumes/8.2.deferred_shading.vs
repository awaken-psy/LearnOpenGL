// Lighting Pass 顶点着色器 —— 与 8.1 相同(NDC 全屏 quad,直接输出位置 + 纹理坐标)
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}