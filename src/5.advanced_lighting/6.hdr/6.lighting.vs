// 场景顶点着色器 — 比第 2 章多了个 inverse_normals 开关(翻转法线)
//
// 主体和第 2 章一样:model 变换位置、法线矩阵(mat3(transpose(inverse(model))))变换法线。
//
// 本 demo 新增 uniform inverse_normals:把法线取反。原因 ——
//   这里渲染的是一个【隧道】:我们站在立方体【里面】看内壁,相机在立方体内部。
//   立方体顶点数据默认法线朝【外】,但我们需要法线朝【内】才能正确照亮内壁,
//   所以 cpp 里对隧道模型传 inverse_normals=true,把法线翻一下。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool inverse_normals;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;

    // inverse_normals=true 时法线取反(隧道内壁:法线要朝内)
    vec3 n = inverse_normals ? -aNormal : aNormal;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalize(normalMatrix * n);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}