// 3.2.blending.vs —— Alpha 混合演示的顶点着色器
// 标准的 MVP 变换，与 3.1 相同。
// 混合在固定管线的片段处理阶段自动完成，着色器无需特殊处理。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}