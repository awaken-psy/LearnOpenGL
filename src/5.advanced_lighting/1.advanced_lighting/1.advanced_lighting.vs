#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// 用【interface block】把片段所需数据打包传给 fs:字段聚合在一起,fs 端用同名块接收(第4章 Advanced GLSL)。
// declare an interface block; see 'Advanced GLSL' for what these are.
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // ⚠ 这里 FragPos 直接取 aPos(物体空间),没乘 model 矩阵。本 demo 地板的 model 是单位矩阵,
    //   所以物体空间 = 世界空间,lightPos/viewPos 都是世界空间,直接用没问题。换场景时记得补 model 变换。
    vs_out.FragPos = aPos;
    vs_out.Normal = aNormal;
    vs_out.TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(aPos, 1.0);
}