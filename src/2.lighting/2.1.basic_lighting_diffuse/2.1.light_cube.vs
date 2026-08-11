// 光源立方体的顶点着色器 — 内容与 1.light_cube.vs 相同(标准 MVP)
// 本课焦点在物体的 basic_lighting shader,光源立方体只是可视化标记,保持最简。
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
