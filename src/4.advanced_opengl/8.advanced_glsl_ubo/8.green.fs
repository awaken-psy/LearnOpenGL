// 片段着色器：输出纯绿色
// 本演示用四个不同颜色的片段着色器区分 4 个立方体。
// 着色器本身非常简单，核心概念在共享的顶点着色器和 UBO 中。
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}