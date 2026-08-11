// 10.2 小行星带顶点着色器（未使用实例化）
// 使用 uniform model 矩阵，每次绘制都需要 CPU 设置不同的 model 矩阵
// ⭐ 这是【非实例化】方式：每颗小行星都需要单独的 draw call

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model; // ⭐ 每次绘制前由 CPU 设置，这是性能瓶颈所在

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0f); 
}
