// 立方体顶点着色器 — 渲染带 2D 纹理的普通立方体
// 位置(3) + 纹理坐标(2)，标准 MVP 变换

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
