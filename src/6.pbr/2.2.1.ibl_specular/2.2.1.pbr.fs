// PBR 片段着色器(2.2.1)—— ⭐ 完整 IBL 之【镜面反射】。
//
// 在 2.1.2(漫反射 IBL)基础上,本 demo 把 ambient 段升级为:
//   ambient = (kD * diffuse + specular) * ao
//   其中:
//     diffuse = irradiance * albedo               (与 2.1.2 相同)
//     specular = prefilteredColor * (F * brdf.x + brdf.y)   ← 本 demo 新增
//
// Split-Sum 应用:
//   1. prefilteredColor = textureLod(prefilterMap, R, roughness*4)  // 按 roughness 选 mip
//   2. brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg           // BRDF 积分查找
//   3. F = fresnelSchlickRoughness(NdotV, F0, roughness)            // 用【粗糙感知】的菲涅尔
//   4. specular = prefilteredColor * (F * brdf.x + brdf.y)
//
// ⭐ 新增 fresnelSchlickRoughness:与 fresnelSchlick 的差别是,用 max(1-roughness, F0) 替代 1,
//   让粗糙表面的菲涅尔在 grazing 角不会过强(粗糙表面 grazing 角不应该有镜面般的强反射)。
//
// Cook-Torrance 直接光照部分与之前 demo 完全相同,不重复注释。
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
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
// ⭐【新增】fresnelSchlickRoughness:粗糙感知的菲涅尔近似。
//   与 fresnelSchlick 的区别:用 max(vec3(1-roughness), F0) 替代常量 1.0。
//   物理意义:粗糙表面 grazing 角的菲涅尔反射不应该像镜面那样飙升到 1.0——
//   roughness 越大,菲涅尔的上限越低,公式里用 1-roughness 体现这一点。
//   直观类比:粗糙的黑塑料 grazing 角反射不了多少光,但光滑的金属镜面就能。
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{		
    vec3 N = Normal;
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

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
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (we now use IBL as the ambient term)
    // ⭐【本 demo 新增】完整的 IBL ambient,拆成 diffuse + specular 两项。
    // 用 fresnelSchlickRoughness(注意是 roughness 版本,不是普通 fresnelSchlick)算菲涅尔,
    // 这样粗糙表面在 grazing 角的菲涅尔不会过强。
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    // 【漫反射 IBL】(与 2.1.2 相同):按法线 N 采样 irradianceMap。
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    // ⭐【镜面反射 IBL】Split-Sum 应用,三步:
    const float MAX_REFLECTION_LOD = 4.0;
    //   (1) 按反射向量 R + roughness 选 mip 采样 prefilterMap。
    //       textureLod:强制用指定 LOD 采样(prefilterMap 已按 roughness 烘焙到不同 mip)。
    //       roughness * 4 把 [0,1] 映射到 [0,4] 的 mip 等级。
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    //   (2) 按入参 (NdotV, roughness) 查 BRDF LUT,得到 .rg = (scale, bias)。
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    //   (3) 组合:specular = prefilteredColor * (F * scale + bias)
    //       数学等价于 F0*A + B 这种形式(把 F 替换为完整菲涅尔 F0+...)。
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    // 完整 ambient = 漫反射 + 镜面反射,再乘 ao(环境光遮蔽)。
    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color , 1.0);
}
