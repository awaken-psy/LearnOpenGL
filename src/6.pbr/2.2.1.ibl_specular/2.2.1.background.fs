// 天空盒片段着色器 —— 与 2.1.2 几乎相同,唯一区别:
//   用 textureLod(..., 0.0) 强制使用 mip level 0(原始 512²)。
//
// 原因:本 demo 给 envCubemap 开启了 mipmap 和 GL_LINEAR_MIPMAP_LINEAR,
//   普通 texture() 会自动选 mip,但天空盒是远景,不应该被模糊——所以锁定 mip 0。
#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

void main()
{
    // ⭐ textureLod 强制 LOD=0,避免自动选到高 mip(那会让天空盒变模糊)。
    vec3 envColor = textureLod(environmentMap, WorldPos, 0.0).rgb;

    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2));

    FragColor = vec4(envColor, 1.0);
}
