// 顶点着色器：几何着色器房屋示例
// 接收 2D 位置和 3D 颜色，通过 VS_OUT 接口块将颜色传递给几何着色器。
// 使用接口块（interface block）是因为几何着色器需要以数组形式接收输入。
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

// ⭐ 接口块 VS_OUT：将多个变量打包传递给下一阶段着色器
// 在几何着色器中，输入变量会以数组形式出现（因为一个图元包含多个顶点），
// 使用接口块可以让 GS 端以 gs_in[].color 的方式访问，语法更清晰。
out VS_OUT {
    vec3 color;
} vs_out;

void main()
{
    vs_out.color = aColor;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}