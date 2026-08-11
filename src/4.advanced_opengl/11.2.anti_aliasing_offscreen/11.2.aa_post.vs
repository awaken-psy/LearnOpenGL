// 11.2 后处理屏幕四边形顶点着色器
// 将一个铺满屏幕的四边形绘制到默认帧缓冲上
// 顶点坐标直接使用 NDC（标准化设备坐标），无需 MVP 变换

#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}  
