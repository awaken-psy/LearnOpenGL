// 阴影映射(3/3)— 主场景片段着色器:三招修复阴影(斜率 bias + PCF + 超出视锥)
//
// 相对 3.1.2 的 ShadowCalculation,本节加了三处修复(详见 cpp 文件头):
//   ① 【bias】比较前给当前深度减一点偏移,治 shadow acne;
//   ② 【PCF】3×3 共 9 次采样取平均,把硬阴影边软化成 0~1 的渐变;
//   ③ 【oversampling】projCoords.z>1.0 表示在 far_plane 外,直接判为无阴影。
// 主光照部分(Blinn-Phong)和 3.1.2 相同,不重复注释。
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 第1步:透视除法(同 3.1.2)。正交投影 w=1 无影响,透视投影必做。
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 第2步:NDC[-1,1] → 纹理坐标[0,1]。
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    // ⭐【修复①:斜率自适应 bias】
    //   法线越偏离光源(掠射),shadow acne 越严重,bias 要越大;
    //   法线正对光源时 dot≈1,bias≈0.005(下限)。
    //   公式:bias = max(0.05 × (1 − dot(normal, lightDir)), 0.005)
    //   不写死一个常数,是为了在不同朝向的表面都刚好盖住失真、又不漏光。
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // ⭐【修复②:PCF(Percentage-Closer Filtering)】——3×3 九次采样取平均,软化硬边。
    //   textureSize(shadowMap, 0) 返回该纹理(0 级 mipmap)的ivec2宽高;
    //   1.0 / textureSize = 每个纹素在 UV 空间的大小 texelSize,用来做邻域偏移。
    //  如果深度贴图是 1024×1024，返回 vec2(1024.0, 1024.0)
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            // 在当前 UV 周围偏移一个纹素,逐个采样深度,各判一次"是否在阴影",
            // 注意每次比较都带上 bias(治 acne),9 次结果累加后再除 9 取平均。
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    // ⭐【修复③a】projCoords.z > 1.0 说明片段在光源 far_plane 之外(深度图根本没覆盖它)。
    //   这类片段不该被误判成阴影,强制 shadow=0。(配套 cpp 端 GL_CLAMP_TO_BORDER。)
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
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
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
}