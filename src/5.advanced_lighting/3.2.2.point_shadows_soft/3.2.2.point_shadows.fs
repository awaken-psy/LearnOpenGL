// 主片段着色器 — ⭐ 在 3.2.1 硬阴影基础上,加【球面 PCF】实现软阴影
//
// 软阴影的直觉:真实光源有面积,阴影边缘是【半影】渐变,不是一刀切。
// 模拟方法 = 【球面 PCF】(Percentage-Trigger Filtering on a disk):
//   不再只采 1 个点硬判"在不在阴影里",而是在【采样方向】周围取多个偏移点,
//   每个点各判一次,最后取【平均值】 → 介于 0~1 的浮点阴影系数 → 边缘柔和过渡。
//
// 与 3.2.1 的差异【只在 ShadowCalculation 函数】:
//   - 3.2.1:texture(depthMap, fragToLight).r,单点采样,shadow ∈ {0, 1}。
//   - 3.2.2:for 20 次 texture(depthMap, fragToLight + 偏移方向×半径).r,
//     累加后 / 20,shadow ∈ [0, 1],边缘半影。
//
// 其他部分(采样方向 = fragToLight、closestDepth × far_plane 还原、Blinn-Phong 光照)
// 与 3.2.1 完全相同,详见 3.2.1.point_shadows.fs。
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;  // 与 3.2.1 相同:用方向向量采样的深度cubemap

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;


// ⭐【球面采样方向数组 gridSamplingDisk[20]】—— 预定义 20 个偏移方向
//   这 20 个方向来自单位立方体的:8 个顶点(全 ±1) + 12 条棱的中点(两个 ±1,一个 0),
//   在球面上分布相对均匀,用作"采样圆盘"上的离散采样点。
//   ⚠ 这里存的只是【方向】(未归一化),实际偏移大小 = 方向 × diskRadius。
// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 fragPos)
{
    // 与 3.2.1 相同:采样方向 = 片段 - 光源。
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;

    // 以下被注释掉的代码是教程演示过的两种写法(单点采样 + 立方体均匀网格 PCF),
    // 教程最终选用下面的 gridSamplingDisk[20] 方案 —— 球面分布更均匀、采样数更少。
    // use the fragment to light vector to sample from the depth map
    // float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // PCF
    // float shadow = 0.0;
    // float bias = 0.05;
    // float samples = 4.0;
    // float offset = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    // {
        // for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        // {
            // for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            // {
                // float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; // use lightdir to lookup cubemap
                // closestDepth *= far_plane;   // Undo mapping [0;1]
                // if(currentDepth - bias > closestDepth)
                    // shadow += 1.0;
            // }
        // }
    // }
    // shadow /= (samples * samples * samples);

    // ⭐ 最终采用的【球面 PCF】方案
    float shadow = 0.0;
    // ⚠ bias = 0.15,比 3.2.1 的 0.05 大 3 倍:多点采样对深度误差更敏感,
    //   偏移太小会出现大量 self-shadow 瑕疵(片元误判自己挡自己)。
    float bias = 0.15;
    int samples = 20;
    // 【diskRadius 随距离自适应】—— 远处的片元用更大的采样半径:
    //   viewDistance 是【相机】到片元的距离(注意不是光源到片元),
    //   越远的片元,屏幕上一个像素覆盖的世界空间越大,用更大的采样圆盘更合理,
    //   让远处阴影更柔和。这是 PCSS(物理软阴影)思路的简化版。
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    // 遍历 20 个方向,在每个方向上各采样一次,累加阴影值。
    for(int i = 0; i < samples; ++i)
    {
        // 在 fragToLight 方向上偏移 gridSamplingDisk[i] × diskRadius,再采样。
        //   相当于在球面采样点周围的小圆盘上取一个深度值。
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1] —— 与 3.2.1 相同的还原步骤
        // 该方向上"当前片元比最近遮挡物远" → 在阴影里,shadow +1。
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    // 取平均 → shadow ∈ [0,1]。0 = 完全照亮,1 = 完全阴影,中间值 = 半影。
    // 这个浮点系数让阴影边缘自然渐变,不再是一刀切。
    shadow /= float(samples);

    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

    return shadow;
}

void main()
{
    // 下面 Blinn-Phong 光照与 3.2.1 完全相同,不再重复注释。
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    // calculate shadow —— shadow 现在是 [0,1] 的浮点数,边缘柔和。
    float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
}