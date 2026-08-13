// 立方体顶点着色器 —— 【与 2.1.1.cubemap.vs 相同】。详见 2.1.1 注释。
// 给 capture pipeline 用:把 WorldPos 作为方向向量传给 fs。
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