// 立方体片元着色器 — 采样 2D 纹理输出颜色
// 与之前的纹理 demo 完全相同，这里立方体贴图只在天空盒中使用

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    FragColor = texture(texture1, TexCoords);
}
