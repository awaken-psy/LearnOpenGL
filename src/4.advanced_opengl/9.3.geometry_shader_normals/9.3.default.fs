// 片段着色器：默认模型渲染
// 对模型漫反射纹理进行采样，用于正常渲染 Backpack 模型。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoords);
}
