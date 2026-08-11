// 2.stencil_testing.fs —— 模板测试演示的片段着色器（正常渲染用）
// 直接采样纹理输出，模板测试在光栅化阶段由固定管线处理，片段着色器无需感知。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    FragColor = texture(texture1, TexCoords);
}