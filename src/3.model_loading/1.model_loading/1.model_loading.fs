// 模型加载的片段着色器 — 只采样漫反射纹理,无光照计算
// texture_diffuse1 是 Mesh::Draw() 按命名约定自动绑定的纹理 uniform。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoords);
}
