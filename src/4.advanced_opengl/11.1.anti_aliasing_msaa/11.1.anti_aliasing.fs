// 11.1 MSAA 抗锯齿片段着色器
// 输出纯绿色——用于观察立方体边缘的抗锯齿效果

#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}  
