// 物体的顶点着色器 — 相对 3.1 新增:传递【纹理坐标 TexCoords】给 fs
// diffuse 贴图需要每个片段知道自己对应的纹理位置,所以 vs 把 aTexCoords 原样传出去。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;   // 新增:纹理坐标属性

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;   // 新增:传给 fs 的纹理坐标

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;   // 纹理坐标不做矩阵变换(它是纹理图上的 2D 位置,不是空间坐标)

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
