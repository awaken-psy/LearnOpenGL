// 物体的片段着色器 — Gouraud 着色:直接用 vs 传来的插值颜色
#version 330 core
out vec4 FragColor;

in vec3 LightingColor;

uniform vec3 objectColor;

void main()
{
    FragColor = vec4(LightingColor * objectColor, 1.0);
}
