// 1.1.depth_testing.fs —— 深度测试演示的片段着色器
// 直接采样纹理颜色输出，不涉及任何深度计算。
// 深度测试在【光栅化阶段】由 OpenGL 固定管线自动处理，无需在着色器中手动操作。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    FragColor = texture(texture1, TexCoords);
}