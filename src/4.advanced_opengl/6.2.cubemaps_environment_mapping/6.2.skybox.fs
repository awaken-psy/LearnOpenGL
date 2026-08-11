// 天空盒片元着色器 — 与 6.1 完全相同
// 用方向向量采样立方体贴图，输出环境颜色

#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}
