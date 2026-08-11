// 片段着色器：爆炸效果
// 对模型纹理进行采样，与普通模型渲染的片段着色器相同。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoords);
}
