// Geometry Pass 顶点着色器 — 把 backpack 的顶点变换到【世界空间】输出给 fs
//
// 注意:FragPos 和 Normal 都输出【世界空间】(不是 view space)。
// 后面 lighting pass 的光源 Position 也是世界坐标,两者要对得上才能算光照方向。
// (对比 9.ssao 的 geometry pass 用的是 view space,两套方案都行,只要统一。)
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz; 
    TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * aNormal;

    gl_Position = projection * view * worldPos;
}