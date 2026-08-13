// SSAO Geometry Pass 顶点着色器 — ⭐ 与 8.1 的关键差异:输出【view space】位置/法线
//
// 注意 FragPos = (view * model * aPos).xyz,是【观察空间】(相机在原点),不是世界空间。
// 后面 SSAO 和 lighting pass 都在 view space 算,坐标系统一。
//
// invertedNormals:房间 cube 的开关。相机在 cube 内部,内表面法线要翻转才朝向相机,
//   传 true 时 aNormal 取反。这个 demo 用一个 uniform bool 控制两种情况。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform bool invertedNormals;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 viewPos = view * model * vec4(aPos, 1.0);
    FragPos = viewPos.xyz;
    TexCoords = aTexCoords;

    // 法线矩阵 = transpose(inverse(M)):处理非均匀缩放,保证法线方向正确(第 2 章已学)。
    // 这里 M = view * model(因为法线也要转到 view space)。
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    Normal = normalMatrix * (invertedNormals ? -aNormal : aNormal);

    gl_Position = projection * viewPos;
}