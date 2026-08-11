// 天空盒片元着色器 — 用方向向量采样立方体贴图
// samplerCube 接受 vec3 方向向量进行采样，返回对应方向的纹素颜色

#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}
