// 天空盒片段着色器 —— 采样 envCubemap(已经从 HDR 转好的立方体贴图)。
//
// ⭐ 注意 envCubemap 是 HDR 浮点纹理,采样值可能远大于 1.0,直接显示会一片白。
//   所以必须做两步处理(和所有 PBR 输出一样):
//     1. Reinhard tonemapping: color / (color + 1) —— 把 (0, ∞) 压到 (0, 1)
//     2. gamma 校正:pow(color, 1/2.2) —— 显示器是 sRGB,线性值要开 0.45 次方才看起来对
#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

void main()
{
    // ⭐ samplerCube:立方体采样器。采样时用 3D 方向向量(不需要 UV),
    //   GPU 自动从对应的面取像素。这里 WorldPos 是立方体顶点位置 = 方向。
    vec3 envColor = texture(environmentMap, WorldPos).rgb;

    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2));

    FragColor = vec4(envColor, 1.0);
}
