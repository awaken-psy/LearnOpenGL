// IBL 镜面反射 Split-Sum 近似【第二部分】:BRDF LUT(查找表)。
//
// 任务:把 BRDF 的【几何 + 菲涅尔】部分(不含光照)预积分成一张 2D 查找表。
//
// 入参(每个像素对应一个 (NdotV, roughness) 组合):
//   TexCoords.x = NdotV(法线·视线, [0,1])
//   TexCoords.y = roughness([0,1])
//
// 出参(RG 两通道):
//   R = A = scale  —— 菲涅尔的"缩放系数"
//   G = B = bias   —— 菲涅尔的"偏置值"
//
// 运行时组合方式:
//   F_ibl = F0 * A + B  ⇒  specular_ibl = prefilteredColor * (F0_scaled + bias)
//   这就是 Split-Sum:prefilter.fs 算了 ∫L_in,这里算了 ∫BRDF,两者相乘近似原积分。
//
// ⭐【IBL 用的几何函数 k 不同!】
//   直接光:k = (roughness + 1)² / 8
//   IBL   :k = roughness² / 2          ← 更小的 k,因为 IBL 的"虚拟光源"分布更广
//   详见 GeometrySchlickGGX 注释。
#version 330 core
out vec2 FragColor;
in vec2 TexCoords;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Hammersley 序列(与 prefilter.fs 完全相同,详见 prefilter.fs 注释)。
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits)
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
// GGX 重要性采样(与 prefilter.fs 相同,详见 prefilter.fs 注释)。
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
// ----------------------------------------------------------------------------
// ⭐【IBL 版】Schlick-GGX 几何函数。
//   与 pbr.fs 直接光版的区别:【k 的公式不同】。
//     直接光:k = (roughness + 1)² / 8
//     IBL   :k = roughness² / 2          (更小)
//   原因:直接光是"点状"光源, grazing 角遮挡更明显;
//        IBL 是"包围"物体的环境光,平均遮挡更弱——用更小的 k 反映这一点。
// note that we use a different k for IBL
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
// Smith 几何函数:直接光和 IBL 共用,但调用上面【IBL 版】的 GeometrySchlickGGX。
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
// ⭐【核心】BRDF 积分:输入 (NdotV, roughness),返回 (A, B) = (scale, bias)。
vec2 IntegrateBRDF(float NdotV, float roughness)
{
    // 构造视线向量 V:把它放在 X-Z 平面里,z 分量 = NdotV,x = sin(acos(NdotV)) = √(1-NdotV²)。
    //   这样 dot(N, V) = NdotV,N 设为 (0,0,1)。
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    // A = 累加 scale,B = 累加 bias。
    float A = 0.0;
    float B = 0.0;

    // 把 N 固定在 (0,0,1),让积分完全在【切线空间】做——结果与方向无关,只取决于 NdotV。
    vec3 N = vec3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the
        // preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        // 由 halfway H + 视线 V 反推出射方向 L。
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        // N、H、V 都在切线空间,z 分量就是 NdotL / NdotH。
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            // ⭐ G_Vis:几何函数的【可见性修正】= G * VdotH / (NdotH * NdotV)。
            //   这是把 Cook-Torrance 的 G/(4·NdotV·NdotL) 重新整理后的形式,
            //   把 4·NdotL 移到了外面,只剩 G·VdotH / (NdotH·NdotV)。
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            // Fc = 菲涅尔的"0~1 渐变值":pow(1 - VdotH, 5)。
            //   VdotH=1 时 Fc=0(正对表面,无菲涅尔);VdotH→0 时 Fc→1(grazing,强菲涅尔)。
            float Fc = pow(1.0 - VdotH, 5.0);

            // ⭐ 关键累加:
            //   A += (1 - Fc) * G_Vis  —— 菲涅尔"低"的部分(scale)
            //   B += Fc * G_Vis        —— 菲涅尔"高"的部分(bias)
            //   最终 F0 * A + B 就等于完整积分(把 F0 留在运行时填)。
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}
// ----------------------------------------------------------------------------
void main()
{
    // 入参直接用 TexCoords:像素的 UV 坐标 = (NdotV, roughness)。
    vec2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
    FragColor = integratedBRDF;
}