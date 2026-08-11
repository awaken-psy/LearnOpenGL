// 光源立方体的片段着色器 — 内容与 1.light_cube.fs 相同(纯白输出)
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0); // 纯白 (1,1,1,1) — set all 4 vector values to 1.0
}
