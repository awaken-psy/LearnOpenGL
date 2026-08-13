// 占位 demo 的顶点着色器 —— 最基础的位置 + 纹理坐标变换,没有任何 CSM 相关逻辑
// (本 demo 本身是空壳,见 csm.cpp 顶部说明)
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoords = texCoords;
}