// 法线贴图的顶点着色器 — ⭐ 把光照所需的三个位置全部转到【切线空间】
//
// 思路:法线贴图里的法线是切线空间下表达的,没法直接和世界空间的光向量点乘。
// 所以这里构造 TBN 矩阵(切线空间的基向量作为列),把 lightPos / viewPos / fragPos
// 统一变换到切线空间再传给 fs,fs 那边就全是切线空间内的运算了。
//
// ⚠ 本 demo 的 model 不含非均匀缩放,理论上直接 mat3(model) 也凑合用,但这里依旧
//    老老实实用法线矩阵 + Gram-Schmidt 重正交化,得到严格正交的 TBN——更稳妥的写法。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// 【接口块 interface block】:把一组 varying 打包传给 fs,fs 那边用同名块接收。
// 这里除了常规的 FragPos/TexCoords,关键是三个 Tangent*——已转到切线空间的位置。
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
    
    // 法线矩阵:与第 2 章同,处理 model 含非均匀缩放时法线方向不被扭歪。
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    // 切线 T、法线 N 都用法线矩阵变换到世界空间。
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    // ⭐【Gram-Schmidt 重正交化】:顶点里的 tangent 只是近似垂直于 N,
    //   减去 T 在 N 方向的投影分量,保证 T 严格 ⊥ N。
    T = normalize(T - dot(T, N) * N);
    // 副切线 B 直接由 N×T 叉乘得出,三者构成正交基。
    vec3 B = cross(N, T);

    // ⭐ mat3(T, B, N) 的列就是切线空间的三个基向量(世界空间下表达),
    //   它能把【切线空间向量变到世界空间】。我们要反过来(世界→切线),
    //   而正交矩阵的逆 = 转置,所以这里取 transpose。
    mat3 TBN = transpose(mat3(T, B, N));
    // 把 lightPos/viewPos/FragPos 一次性转到切线空间——fs 拿到后直接做光照。
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
        
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}