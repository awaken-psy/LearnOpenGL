// 视差贴图系列的顶点着色器 — 和 4.normal_mapping.vs 目标相同(把位置转到切线空间)
//
// ⚠ 与 4 的差异(更省、但精度略低):
//   - 不算法线矩阵,直接用 mat3(model) 变换 T/B/N(model 这里无非均匀缩放,够用)
//   - 不做 Gram-Schmidt 重正交化——T 直接来自顶点属性
//   - bitangent 也作为【顶点属性 aBitangent】传入,而不是用 cross(N,T) 现算
//   三个向量各自 normalize 后塞进 TBN,代码更短更快,代价是 T/B/N 不严格正交。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = aTexCoords;   
    
    // T/B/N 全部直接用 mat3(model) 变换 + normalize,不做重正交化(与 4 的关键差异)。
    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);
    vec3 N = normalize(mat3(model) * aNormal);
    // 正交矩阵的逆 = 转置,所以 transpose(TBN) 把世界空间位置变到切线空间。
    mat3 TBN = transpose(mat3(T, B, N));

    // 同 4.normal_mapping:三个关键位置一次性转到切线空间。
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}