// 10.3 行星片段着色器
// 采样漫反射纹理，输出行星表面颜色

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoords);
}
