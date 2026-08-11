// 2.stencil_single_color.fs —— 模板描边的纯色片段着色器
// ⭐ 用于第二遍渲染：输出固定纯色作为描边颜色。
// 只在模板测试通过的区域（即物体轮廓边缘）被绘制。
// 这个颜色值 (0.04, 0.28, 0.26) 是一种深青绿色，形成明显的轮廓边框。
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}