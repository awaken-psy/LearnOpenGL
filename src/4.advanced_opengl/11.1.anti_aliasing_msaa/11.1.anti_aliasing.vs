// 11.1 MSAA 抗锯齿顶点着色器
// 标准的 MVP 变换，与之前的顶点着色器无异
// 抗锯齿在光栅化阶段自动完成，着色器无需特殊处理

#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
