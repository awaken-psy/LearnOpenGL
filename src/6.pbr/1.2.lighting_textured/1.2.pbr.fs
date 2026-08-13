// 片段着色器 — ⭐【纹理化 PBR】直接光照版(在 1.1 基础上把 uniform 材质换成纹理)
//
// 与 1.1 的区别(只有材质来源和法线变了,Cook-Torrance 公式完全相同):
//   - 材质从 4 个 uniform → 5 张【PBR 纹理】:albedo / normal / metallic / roughness / ao
//   - albedo 纹理是 sRGB 空间,采样后要【pow(.,2.2) 转线性】再参与光照计算
//   - 法线来自【法线贴图】,但用 getNormalFromMap() 的 trick 在片元导数空间现场重建 TBN,
//     免去预计算切线(详见该函数注释)
//   - metallic / roughness / ao 都是单通道图,数据本身线性,直接从 .r 读取
//
// ⚠ 下方 D/G/F 三个函数和 1.1 一模一样,不再重复注释。只标 main() 里的差异。
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// 材质参数改成 5 张【PBR 纹理】(metallic-roughness 工作流)
// material parameters
uniform sampler2D albedoMap;       // 反照率(sRGB,采样后需转线性)
uniform sampler2D normalMap;       // 切线空间法线
uniform sampler2D metallicMap;     // 金属度(单通道)
uniform sampler2D roughnessMap;    // 粗糙度(单通道)
uniform sampler2D aoMap;           // 环境遮蔽(单通道)

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// ⭐ 从法线贴图取法线,并用【片元导数 dFdx/dFdy】现场重建 TBN 矩阵——免预计算切线。
// 这是简化 PBR 教程的【easy trick】:常规做法是在顶点着色器里用预计算的切线属性算 TBN
// (见第 5 章法线贴图),这里用屏幕空间导数偷懒:
//   - dFdx(WorldPos) / dFdy(WorldPos):相邻片元在世界空间的位置差,给出表面的两个切向
//   - 配合 dFdx(TexCoords) / dFdy(TexCoords) 的 UV 差,解出 T 和 B 的方向
//   - dFdx(p) / dFdy(p):GLSL 内建函数,返回变量 p 在屏幕 x/y 方向相邻片元间的变化率
// 这样就不用在顶点数据里存切线了。⚠ 性能不如常规预计算切线路径,生产代码请走常规路径。
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal
// mapping the usual way for performance anyways; I do plan make a note of this
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    // 法线贴图存的是 [0,1],映射回 [-1,1] 得到切线空间法线
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    // 用屏幕空间导数重建 TBN:位置和 UV 的片元间变化
    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);   // 由位置差和 UV 差解出切线方向
    vec3 B  = -normalize(cross(N, T));          // 副切线(叉乘补出第三个轴)
    mat3 TBN = mat3(T, B, N);                   // 切线空间 → 世界空间

    return normalize(TBN * tangentNormal);      // 把切线空间法线转到世界空间
}
// ----------------------------------------------------------------------------
// 【D】【G】【F】三项与 1.1.pbr.fs 完全相同,不再注释。
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{
    // ⭐ 材质从纹理采样。albedo 是 sRGB → 必须【pow(.,2.2) 转线性】再算光照,否则偏亮。
    vec3 albedo     = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    // metallic/roughness/ao 都是单通道图,数据本身就是线性的,直接取 .r 即可。
    float metallic  = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao        = texture(aoMap, TexCoords).r;

    vec3 N = getNormalFromMap();                // 法线从贴图取(1.1 是用顶点法线)
    vec3 V = normalize(camPos - WorldPos);

    // F0 同 1.1:非金属 0.04,金属用 albedo。
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // —— 以下 Cook-Torrance 反射率循环、能量守恒、tonemap、gamma 全部与 1.1 相同,不再注释 ——
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}