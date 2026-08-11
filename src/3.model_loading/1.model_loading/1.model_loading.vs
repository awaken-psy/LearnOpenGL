// 模型加载的顶点着色器 — 标准 MVP 变换 + 传递纹理坐标
// 本课焦点在模型加载流程,shader 极简(无光照,只采样纹理)。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
