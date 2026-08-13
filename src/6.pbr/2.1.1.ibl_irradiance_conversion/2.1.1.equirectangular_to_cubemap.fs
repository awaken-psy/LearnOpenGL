// 把 HDR 全景图(equirectangular 球面投影)采样的着色器。
//
// 用途:capture pipeline 里渲染单位立方体,每个屏幕像素的方向(WorldPos)对应一个
//      立方体面上的方向。本 fs 把这个方向转成球面 UV,去 2D HDR 图采样,写到 cubemap。
//
// ⭐【3D 方向 → 2D 球面 UV】的数学:
//      球面坐标:经度 φ = atan(z, x)  ∈ [-π, π]   (绕 Y 轴的角度)
//                纬度 θ = asin(y)    ∈ [-π/2, π/2] (上下仰角)
//      归一化到 [0, 1] 的 UV:u = φ / (2π) + 0.5
//                            v = θ / π   + 0.5
//      所以 invAtan = (1/(2π), 1/π) = (0.1591, 0.3183),先乘后加 0.5 就完成映射。
#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D equirectangularMap;

// ⭐ invAtan = (1/(2π), 1/π):把 atan/asin 的弧度结果归一化到 [0, 1] 的预计算常数。
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    // atan(z, x):两参数版 atan2,返回 z/x 的方位角,自动判断象限。范围 [-π, π]。
    // asin(y):由 y(归一化后即 sin(θ))反推仰角。范围 [-π/2, π/2]。
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;    // 弧度 → [0, 1]
    uv += 0.5;        // 平移到 UV 中心
    return uv;
}

void main()
{
    // ⭐ normalize(WorldPos):cubemap 顶点位置作为方向向量,先归一化(方向无所谓长度)。
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;

    FragColor = vec4(color, 1.0);
}
