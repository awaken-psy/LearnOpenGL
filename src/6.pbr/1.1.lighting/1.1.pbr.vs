// 顶点着色器 — PBR 直接光照
// 和之前章节几乎一样,唯一区别:PBR 在【世界空间】计算光照,
// 所以把世界坐标 WorldPos 和法线 Normal 都传给片段着色器(WorldPos 用来算光照方向/距离)。
// normalMatrix = mat3(transpose(inverse(model))),处理非均匀缩放下的法线(见 2.2)。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

void main()
{
    TexCoords = aTexCoords;
    WorldPos = vec3(model * vec4(aPos, 1.0));   // 世界空间坐标(给 fs 算光照方向、距离)
    Normal = normalMatrix * aNormal;            // 世界空间法线(已处理非均匀缩放)

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}