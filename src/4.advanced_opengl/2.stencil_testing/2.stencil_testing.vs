// 2.stencil_testing.vs —— 模板测试演示的顶点着色器
// 与标准顶点着色器相同，传递位置和纹理坐标。
// 模板测试发生在片段着色器之后的固定管线阶段，顶点着色器无需特殊处理。
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
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}