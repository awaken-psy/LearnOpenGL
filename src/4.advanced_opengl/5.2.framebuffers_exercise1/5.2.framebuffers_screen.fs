// 镜子四边形片元着色器 — 与 5.1 screen 着色器相同
// 采样 FBO 中镜像视角的纹理，贴到镜子四边形上

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
} 
