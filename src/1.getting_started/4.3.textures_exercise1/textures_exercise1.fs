// 练习1片段着色器（孤立的参考答案，未纳入构建）
//
// 焦点：在采样第二张纹理时翻转 X 坐标，让笑脸左右镜像。
// vec2(1.0 - TexCoord.x, TexCoord.y) — 把 u 坐标 0↔1 对调，相当于水平翻转纹理。
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

void main()
{
    // texture2 采样时用 1.0 - TexCoord.x 翻转横向坐标 → 笑脸朝反方向
    FragColor = mix(texture(ourTexture1, TexCoord), texture(ourTexture2, vec2(1.0 - TexCoord.x, TexCoord.y)), 0.2);
}
