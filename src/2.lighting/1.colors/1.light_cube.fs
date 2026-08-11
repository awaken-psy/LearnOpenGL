// 光源立方体的片段着色器 — 直接输出纯白
// 光源本身就是光的来源,不需要被光照计算,固定发白光,纯粹是个可视化标记
// (让你在场景里"看见"光在哪儿)。
#version 330 core
out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
    FragColor = vec4(lightColor, 1.0);
}
