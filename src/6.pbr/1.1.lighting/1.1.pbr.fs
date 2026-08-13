// 片段着色器 — ⭐【PBR 基于物理的渲染】直接光照版
//
// 前 5 章我们用 Blinn-Phong 凑合出"看起来还行"的光照,但它不真实——
// 高光形状、能量、金属/非金属的差别全靠经验参数硬调,换个环境就穿帮。
// PBR 用符合【物理规律】的公式描述光与表面的交互,让材质参数有真实含义,
// 在任何光照环境下都能给出正确结果。
//
// 核心:用【Cook-Torrance BRDF】替换原来的漫反射+高光经验公式:
//   Lo += ( 漫反射BRDF + 镜面BRDF ) × 入射辐射度 × NdotL
//     漫反射BRDF = kD × albedo / π                 (兰伯特漫反射,被 kD 调节)
//     镜面BRDF   = D × G × F / (4·NdotV·NdotL)     (微表面模型,三项相乘)
//       【D】法向分布函数 — 多少比例的微表面正好对齐半程向量 H(决定高光的形状和大小)
//       【G】几何函数     — 多少微表面被自己挡住(自阴影/遮蔽),粗糙面更严重
//       【F】菲涅尔项     — 反射率随观察角变化(掠射角反射更强,正面反射较弱)
//
// 【metallic-roughness 工作流】用两个标量描述材质:
//   metallic  (0=非金属/电介质, 1=纯金属)  决定有无漫反射、F0 取什么
//   roughness (0=镜面光滑, 1=完全粗糙)     喂给 D 和 G,控制高光散开程度
//
// 物理上的光强不用 0~1 的"颜色",而用【辐射度 radiance】(单位面积能量)。
// 这里 4 盏灯 radiance = 300,配合【平方反比衰减】1/distance²——值很大,
// 所以最后必须 Reinhard tonemap 把 HDR 压回 0~1 再做 gamma 校正。
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// 材质参数(本 demo 用 uniform,1.2 节会换成纹理)——【metallic-roughness 工作流】
// material parameters
uniform vec3 albedo;      // 反照率:物体本身的"颜色"(非金属用它当漫反射色,金属用它当 F0)
uniform float metallic;   // 金属度:0=非金属(塑料/木材),1=纯金属(铜/铁),中间值是混合
uniform float roughness;  // 粗糙度:0=镜面,1=粗糙。直接喂给 D 和 G 函数
uniform float ao;         // 环境遮蔽:本节用不到(仅乘到很弱的环境光上)

// lights
uniform vec3 lightPositions[4];  // 4 盏点光源位置
uniform vec3 lightColors[4];     // 4 盏点光源的辐射度(物理单位,不是颜色)

uniform vec3 camPos;      // 相机位置(算视线方向 V、菲涅尔用)

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// 【D】法向分布函数 — GGX(又叫 Trowbridge-Reitz)
// 直觉:表面由无数微小镜面组成,这个函数统计"有多少微面的法线正好指向半程向量 H"。
// NdotH 越接近 1(微面正对 H),返回值越大 → 高光中心越亮。
// roughness 越大,峰值越低、尾巴越长 → 高光散开变模糊。
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    // a = roughness²。直接用 roughness 会让材质在中段变化不明显,平方后视觉更线性。
    float a = roughness*roughness;
    float a2 = a*a;                    // a² = roughness⁴
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    // ⭐ GGX 公式:D = a2 / [ π × (NdotH²×(a2−1) + 1)² ]
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
// 【G】几何函数的单边 Schlick-GGX 近似(用于直接光照)。
// 直觉:粗糙表面的微面会互相挡住光(自阴影)或互相挡住反射(自遮蔽)。
// 这是"单方向"的遮挡率:输入某个 NdotV 或 NdotL,返回 0~1 的可见比例。
// ⚠ 直接光照版本的 k = (roughness+1)²/8,和 IBL 版本(roughness²/2)不同,
//   因为直接光是精确计算,IBL 是积分近似,需要的"粗糙度重映射"不一样。
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;             // 直接光照专用 k

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
// 【G】几何函数 — Smith 方法:把视线方向和光线方向各自算一次 SchlickGGX,再相乘。
// 视线方向 V 的遮挡(遮蔽)+ 光线方向 L 的遮挡(阴影),两者结合才是完整遮挡。
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);  // 视线方向的可见率
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);  // 光线方向的可见率

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
// 【F】菲涅尔项 — Schlick 近似。
// 直觉:同样的表面,正对看(法线对着你)反射弱,斜着看(掠射角)反射强——
// 想象水面:低头垂直看能见底(反射少),远处看全是反光(反射强)。
// F0 = 正对看时的基础反射率;cosTheta 通常是 H·V(半程向量与视线的夹角)。
// 返回值随角度从 F0 平滑升到接近 1(近乎全反射)。
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{
    // PBR 在【世界空间】计算:法线 N 和视线方向 V(片段→相机)。
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    // F0 = 正对看时的基础反射率。【非金属】(电介质)F0 几乎都是 0.04(各种塑料/木材差不多),
    // 【纯金属】没有这个固定值,直接用 albedo 当 F0。用 metallic 在两者间线性插值。
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // 反射率方程:逐盏灯累加出射辐射度 Lo
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {
        // ---- 每盏光的入射量 ----
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);  // 光线方向(片段→光源)
        vec3 H = normalize(V + L);                         // ⭐ 半程向量:V 与 L 的角平分线,
                                                           //   微面法线要对齐的就是它
        float distance = length(lightPositions[i] - WorldPos);
        // ⭐ 物理衰减 = 1/distance²(平方反比)。比之前线性衰减真实得多。
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;      // 到达片段的辐射度

        // ---- Cook-Torrance 镜面 BRDF 的三项 D/G/F ----
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);      // 【D】
        float G   = GeometrySmith(N, V, L, roughness);     // 【G】
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);  // 【F】

        // 镜面 BRDF = D·G·F / (4·NdotV·NdotL);+0.0001 防止除零
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // ---- 【能量守恒】:反射出去的光不能比收到的多 ----
        // kS(镜面占比)= 菲涅尔 F:反射多少就剩多少给漫反射。
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;                              // ⚠ 金属无漫反射:metallic=1 时 kD 归零

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);                 // 朝光的面才被照亮

        // ⭐ 累加:(漫反射BRDF kD·albedo/π + 镜面BRDF specular) × 辐射度 × NdotL
        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // 环境光:本节用很弱的常数项凑合(下一节 IBL 会换成真实环境贴图)。
    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // ⭐ 【Reinhard tonemap】:color/(color+1) 把 HDR(可能远大于 1)压回 0~1。
    //   辐射度 300 + 平方反比衰减下,光在近处会爆掉,必须 tonemap 才能正常显示。
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma 校正(sRGB 输出)。这里假设 albedo 已经在线性空间。
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}