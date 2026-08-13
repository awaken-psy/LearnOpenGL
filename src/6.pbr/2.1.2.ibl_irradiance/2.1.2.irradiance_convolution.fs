// IBL 漫反射的核心:对 envCubemap 做【半球卷积】,输出 irradianceMap。
//
// 物理意义:对每个像素(对应一个法线方向 N),把 N 上方【整个半球的入射光】加起来,
//          就是这块表面收到的【总环境光】(irradiance)。运行时按法线采样即可,不用实时算积分。
//
// 数学公式:L_irradiance(N) = ∫_Ω L_in(ω) * cos(θ) dω    (∫ 在上半球 Ω 上)
//   - L_in(ω) = texture(envCubemap, ω) —— 来自 ω 方向的环境光
//   - cos(θ) = Lambert 投影定律(光越斜越弱,法线方向最强)
//   - dω = sin(θ) dθ dφ —— 球面上的微小立体角(球面坐标的雅可比)
//
// 实现策略:黎曼和——在球面坐标 (φ, θ) 上等步长采样,把所有采样值累加,最后除以采样数。
//
// ⚠ 关于最后的乘 π:BRDF 的漫反射项是 albedo/π * irradiance,这里预先把 π 乘进 irradiance,
//   pbr.fs 里就只需要 irradiance * albedo,不用再除 π。这样卷积结果可以直接当 diffuse 颜色用。
#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
	// The world vector acts as the normal of a tangent surface
    // from the origin, aligned to WorldPos. Given this normal, calculate all
    // incoming radiance of the environment. The result of this radiance
    // is the radiance of light coming from -Normal direction, which is what
    // we use in the PBR shader to sample irradiance.
    // 当前 fragment 在 cubemap 某一面上,WorldPos 即为该位置的【法线方向】。
    vec3 N = normalize(WorldPos);

    vec3 irradiance = vec3(0.0);

    // 【建切线空间基】:用 N 构造一组正交向量 (right, up, N),
    // 这样后面就能把"局部球面坐标采到的方向"变换到世界空间去采样 envCubemap。
    // ⚠ 不能直接用世界 (1,0,0)(0,1,0):当 N 恰好与 up 平行(如极点)时叉乘为 0,
    //    要先选定一个不平行于 N 的辅助向量(这里固定 (0,1,0))再叉乘两次得到正交基。
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));

    // sampleDelta=0.025:φ 和 θ 的采样步长(弧度)。
    //   总采样数 = (2π/0.025) * (π/2)/0.025 ≈ 251 * 63 ≈ 15,813 个/像素。
    //   越小越精确,但卷积越慢(32×32×6 面已经很慢,需要约 1 秒)。
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    // 双重循环遍历上半球:φ ∈ [0, 2π) 绕一圈,θ ∈ [0, π/2) 从法线到水平面。
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // 球面坐标 → 笛卡尔坐标(切线空间下,z 轴 = N 方向):
            //   x = sin(θ)cos(φ), y = sin(θ)sin(φ), z = cos(θ)
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // 切线空间 → 世界空间:用 (right, up, N) 三基向量构造的基底变换。
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            // ⭐ 核心累加:每个采样的贡献 = 环境光 * cos(θ) * sin(θ)
            //   cos(θ):Lambert 定律,光越斜贡献越小
            //   sin(θ):【立体角权重】—— θ 越接近 π/2(赤道),单位 dθ dφ 对应的球面面积越大
            //          (赤道附近的"一圈"比极点附近的"一圈"大得多)
            //   缺少 sin(θ) 会让赤道方向被低估,导致卷积结果偏暗。
            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    // ⭐ 最后乘 π / nrSamples:取平均后再乘 π。
    //   乘 π 是为了【抵消】BRDF 里的 1/π 系数,这样 pbr.fs 里 diffuse = irradiance * albedo 就完整。
    //   不除 π 是因为 BRDF 的 albedo/π 已经在物理推导中被消掉——这里预先合并,运行时省一次除法。
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    FragColor = vec4(irradiance, 1.0);
}
