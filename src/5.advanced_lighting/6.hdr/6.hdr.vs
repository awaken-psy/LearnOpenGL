// 全屏四边形顶点着色器(第 4 章学过,无新内容)
// 输入已是 NDC 坐标,直接透传给 gl_Position,把 UV 传给 fs 采样 HDR 纹理。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}