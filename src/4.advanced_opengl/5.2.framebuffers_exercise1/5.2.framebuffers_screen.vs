// 镜子四边形顶点着色器 — 与 5.1 screen 着色器相同
// ⭐ 不使用 MVP 矩阵，顶点坐标直接作为 NDC 输出
//   与 5.1 的区别在于 C++ 端传入的顶点坐标更小（右上角小矩形而非全屏）

#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}	
