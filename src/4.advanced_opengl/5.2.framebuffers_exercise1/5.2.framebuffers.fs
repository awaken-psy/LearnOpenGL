// 场景片元着色器 — 与 5.1 完全相同
// 直接采样纹理颜色输出，无后处理

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    FragColor = texture(texture1, TexCoords);
}
