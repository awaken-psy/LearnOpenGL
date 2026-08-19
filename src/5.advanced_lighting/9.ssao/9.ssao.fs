// ⭐ SSAO 核心片段着色器 —— 算每个像素的环境光遮蔽因子(0=完全遮挡,1=无遮挡)
//
// 算法直觉:在当前 fragment 周围【半球】内撒 64 个采样点,每个采样点投影回屏幕查它的
// 实际深度。如果采样点在几何后面(被挡住),说明这个方向有遮挡。被挡的采样越多,遮蔽越强。
//
// 三个关键技巧:
//   1. 【TBN 矩阵】:把切线空间的采样核转到 view space,让半球始终对齐法线方向。
//      用 Gram-Schmidt 正交化:tangent = normalize(randomVec - normal * dot(randomVec, normal))
//   2. 【噪声纹理平铺】:4×4 旋转向量铺满屏幕,每 4×4 像素一个随机旋转,避免采样模式重复。
//   3. 【rangeCheck smoothstep】:防止远处几何误判为遮挡(采样点和 fragment 深度差太大时不算)。
#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.5;   // 采样半球半径:越大遮蔽范围越广,但更"糊"
float bias = 0.025;   // 深度偏移:防止"自遮蔽"假阴影(表面自己挡自己)

// ⭐ 噪声平铺比例 = 屏幕分辨率 / 噪声纹理尺寸(4×4)。
//   TexCoords * noiseScale 会让噪声每 4×4 像素重复一次,实现全屏平铺。
// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); 

uniform mat4 projection;

void main()
{
    // get input for SSAO algorithm
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    // 采样噪声纹理(平铺后),给 TBN 矩阵引入随机旋转。
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    // ⭐ 构造【TBN 切线空间基矩阵】把采样核从切线空间转到 view space。
    //   tangent 用 Gram-Schmidt 正交化:从 randomVec 减去其在 normal 方向的投影分量,
    //   得到与 normal 垂直的向量。bitangent = normal × tangent。三者组成正交基 TBN。
    //   这样采样核(本来在 +z 半球)会被旋转到【以当前法线为 z 轴】的方向。
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // ⭐ 把采样点从切线空间转到 view space,再加到 fragment 位置上(乘 radius 控制范围)。
        //   samplePos = fragPos 周围半球内的一个 3D 点(view space)。
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // ⭐ 把 samplePos(view space)【投影回屏幕】拿到对应的纹理坐标,这样后面才能
        //   从 gPosition 采样查"这个屏幕位置的实际几何深度"。
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // 从 G-Buffer 采样 offset.xy 处的【实际几何深度】(z 分量)。
        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        // ⭐ rangeCheck:防止远处的几何被误判成遮挡。
        //    smoothstep(0, 1, radius / |fragPos.z - sampleDepth|):
        //   当 x 在 edge0 和 edge1 之间时，返回一个 0 到 1 的平滑过渡值
        //   当采样点深度和 fragment 深度差很大(几何离得远)时,radius/差值→0,rangeCheck→0,
        //   这个采样不计入遮蔽。差值小时 rangeCheck→1,正常计入。
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        // ⭐ 核心判断:sampleDepth >= samplePos.z + bias 表示采样点在几何【后面】(被挡)。
        //   bias 防止自遮蔽假阴影。累加每个被挡的采样点(乘 rangeCheck 权重)。
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    // 取反并归一化:被挡的采样越多 occlusion 越大,取反后变成"遮蔽越强 → 值越小 → ambient 越暗"。
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = occlusion;
}
