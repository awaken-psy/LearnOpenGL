// 立方体顶点着色器 —— 【与 2.1.2.cubemap.vs 相同】。capture pipeline 共用。
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    WorldPos = aPos;  
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}