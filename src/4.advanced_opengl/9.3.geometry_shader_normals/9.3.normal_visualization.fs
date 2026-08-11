// 片段着色器：法线可视化
// 输出固定黄色，使法线线段在深色背景上清晰可见。
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
