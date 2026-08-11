// 1.2.depth_testing.vs —— 深度可视化演示的顶点着色器
// 只需要顶点位置，不需要纹理坐标——因为片段着色器将直接输出深度值对应的灰度色。
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}