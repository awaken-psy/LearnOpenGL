// IBL 镜面反射 Split-Sum 近似【第一部分】:预过滤环境贴图。
//
// 任务:把 envCubemap(由 cpp 每个 mip 调用一次)预过滤成"对应某 roughness 的模糊版本"。
//
// 数学原理(关键):镜面反射积分 ∫ L_in(ωi) * D(H) * V(V,L) dωi 不能解析,
// 但可以用【GGX 重要性采样】——只采那些 GGX 认为重要的方向(围绕反射向量 R 集中)。
// 这样 1024 个样本就够用,而 irradiance 那种均匀采样要上万才达同样精度。
//
// ⭐ 简化假设:R = N, V = R(把反射向量、视线向量都设成法线方向)。
//   这是 Epic Games 的 trick——预计算阶段假装视角是"正对法线看",
//   让结果与视角无关(否则得给每个 (R, V) 组合存一张图,不可能)。
//   代价:在 grazing 角(视角擦边)时会有一定误差,但视觉上几乎察觉不到。
//
// ⭐ 亮点斑消除(adaptive mip):高 roughness 时,采样方向会"散开",
//   可能采到环境贴图里某些像素特别亮的点(如太阳),形成亮点斑。
//   解决方案:按当前采样的 PDF 反算"应该从 envCubemap 的哪个 mip 采样",
//   PDF 越小(采样越偏离中心)就用越高的 mip(越模糊),把亮点糊掉。
//   mipLevel = 0.5 * log2(saSample / saTexel)
#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// GGX 法线分布函数,和 pbr.fs 里完全一样,这里用来估采样方向的"重要性"。
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
// 【Hammersley 序列】:生成 N 个【低差异序列】(quasi-random),
// 比纯随机更均匀地覆盖 [0,1]² 平方——同样样本数下收敛更快。
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
//   RadicalInverse_VdC 是 bit-reverse 技巧:把整数 i 的二进制位反转后当小数。
//   例如 i=6(二进制 110)→ 反转 011 → 0.375。
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
// Hammersley 第 i 个样本(共 N 个):返回 (i/N, RadicalInverse_VdC(i))。
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
// 【重要性采样 GGX】:给定低差异随机数 Xi 和法线 N、roughness,
// 返回一个【按 GGX 分布偏置】的 halfway 向量 H。
//   直觉:roughness 越小,H 越集中在 N 周围(镜面反射);roughness 大时 H 散开(漫反射)。
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;

	float phi = 2.0 * PI * Xi.x;
	// ⭐ GGX 重要性采样的关键公式:把均匀的 Xi.y 变换成 GGX 分布的 cos(θ)
	//   (分母里的 (a²-1)*Xi.y 让分布形状匹配 GGX 的"中间高、边缘低")。
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// 把切线空间下的 H 转到世界空间(用 N 构造正交基)。
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
// ----------------------------------------------------------------------------
void main()
{
    // 当前 fragment 对应的法线方向(由 cpp 设置 roughness 决定使用哪个 mip)。
    vec3 N = normalize(WorldPos);

    // make the simplifying assumption that V equals R equals the normal
    // ⭐ 简化假设:V = R = N。这让预过滤结果与运行时视角无关。
    //   代价是 grazing 角精度损失,但视觉上几乎察觉不到。
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;

    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        // H 是 GGX 重要性采样的 halfway 向量。
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        // ⭐ 由 H 和 V 反推出射方向 L(反射向量公式:reflect(-V, H) = 2*(V·H)*H - V)。
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            // —— 自适应 mip 选择(消除亮点斑)。
            float D   = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            // 【PDF】(probability density function):当前采样方向的重要性,
            //   由 GGX 分布推导出来。pdf = D * NdotH / (4 * HdotV)。
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            // ⭐ 自适应 mip 推导(核心):
            //   saTexel:envCubemap 单个 texel 对应的立体角(球面分辨率)
            //     = 4π / (6 * 512²)(球面总立体角 / 6 面总 texel 数)
            //   saSample:当前样本平均覆盖的立体角
            //     = 1 / (样本总数 * pdf)(PDF 越小,该样本代表的角度范围越大)
            //   mipLevel = 0.5 * log2(saSample / saTexel)
            //     → 如果一个样本代表的立体角比一个 texel 大,就往高 mip 走(取更模糊的像素),
            //       相当于"采样时顺便做了 box filter"——这就是亮点斑的成因和解决方案。
            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            // ⭐ textureLod(sampler, P, lod):强制使用指定 mip level 采样(不做自动 mip 选择)。
            //   普通 texture() 会自动按导数选 mip,这里我们要按 PDF 手动选——所以用 textureLod。
            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    // 按 NdotL 加权平均(不是简单除以样本数),让偏离法线的样本贡献小一些。
    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}
