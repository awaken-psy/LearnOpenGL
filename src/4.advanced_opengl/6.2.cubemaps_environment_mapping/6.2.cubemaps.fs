// 立方体片元着色器 — 反射环境贴图
//
// ⭐ 反射计算三步走：
//   1. I = normalize(Position - cameraPos) — 从摄像机指向片元的【视线方向】
//   2. R = reflect(I, normalize(Normal))   — 根据法线计算【反射向量】
//   3. texture(skybox, R)                   — 用反射向量采样立方体贴图
//
// reflect(I, N) 的物理含义：
//   I 是入射方向（从摄像机到片元），N 是表面法线，
//   reflect 返回 I 关于 N 的镜像方向，即"反射出去的光线方向"。
//   用这个方向去采样环境贴图，就得到了该表面点应该"映出"的环境颜色。

#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{    
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}
