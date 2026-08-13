// 阴影映射(2/3)— 调试 quad 顶点着色器。内容与 3.1.1 完全相同,详见 3.1.1.debug_quad.vs。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}