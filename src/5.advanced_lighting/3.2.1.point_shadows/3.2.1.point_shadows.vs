// 主着色器的顶点着色器 — 把世界空间位置/法线/纹理坐标传给 fs
//
// 新概念:【接口块 interface block】VS_OUT —— 把一组 out 变量打包成一个块,
// fs 端用同名块接收(in VS_OUT {...} fs_in),代码更整洁,也是 GS/抽象层的常见写法。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// reverse_normals:【房间内壁】光照 hack 的开关(详见 cpp 端注释)。
//   1 = 把法线取反,让房间内壁的法线朝内,光照公式才正确;0 = 正常。
uniform bool reverse_normals;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    // 法线矩阵 = mat3(transpose(inverse(model))) (第2章学过)。
    // ⭐ 这里根据 reverse_normals 决定是否把 aNormal 取反:房间内壁专用 hack。
    if(reverse_normals) // a slight hack to make sure the outer large cube displays lighting from the 'inside' instead of the default 'outside'.
        vs_out.Normal = transpose(inverse(mat3(model))) * (-1.0 * aNormal);
    else
        vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}