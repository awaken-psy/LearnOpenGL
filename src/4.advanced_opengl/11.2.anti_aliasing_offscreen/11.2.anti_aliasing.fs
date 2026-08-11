// 11.2 离屏 MSAA 场景片段着色器
// 输出纯绿色——渲染到多采样帧缓冲中

#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}  
