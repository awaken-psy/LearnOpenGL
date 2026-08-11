// 物体的顶点着色器 — 标准的 MVP 变换(和第一章相同,无纹理坐标)
// 本课焦点在片段着色器 1.colors.fs 的颜色相乘,这里只负责把顶点变换到屏幕。
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
