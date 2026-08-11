// 场景片元着色器 — 采样纹理颜色输出到 FBO
// 最基础的纹理采样：直接输出纹理颜色，不做任何后处理
// 后续练习中后处理逻辑会加在 screen 着色器里，而非这里

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    FragColor = texture(texture1, TexCoords);
}
