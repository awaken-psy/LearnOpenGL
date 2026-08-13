// 阴影映射(2/3)— 主场景片段着色器:【ShadowCalculation】核心阴影判断
//
// 整体流程:Blinn-Phong 算光照 → 调 ShadowCalculation 得到阴影因子(0=lit,1=shadowed)
// → 阴影部分只留环境光:lighting = (ambient + (1−shadow)×(diffuse+specular)) × color。
//
// ⚠ 本节【故意不加 bias】,所以能看到明显的【阴影失真 shadow acne】(黑白条纹)。
//   原因:深度图纹素有尺寸,一片"平面"上每个纹素采到的最近深度有微小抖动,
//   导致平面对自己的片段时而"在阴影里"时而"不在"。下一节加 bias 推开一点就好。
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;   // 第一趟烤好的深度图

uniform vec3 lightPos;
uniform vec3 viewPos;

// ⭐【阴影判断函数】输入片段在光源空间的位置,返回 0(被照亮)或 1(在阴影里)。
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    // 第1步【透视除法】:齐次坐标 → 笛卡尔,把 w 除掉。正交投影 w=1 无影响,
    // 但透视投影的深度图必须做。结果落在【裁剪空间 NDC [-1,1]】。
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    // 第2步:NDC 的 [-1,1] 映射到纹理坐标的 [0,1](因为纹理 UV 是 0..1)。
    //   公式:dst = src × 0.5 + 0.5。现在 projCoords.xyz 都在 [0,1]。
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    // 第3步:用 projCoords.xy 当 UV 采样深度图,得到【光源能看到的最近深度】。
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    // 第4步:当前片段在光源空间下的深度(就是 projCoords.z)。
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    // ⭐ 第5步【核心比较】:当前片段比"最近深度"还深 → 被前面挡住了 → shadow=1。
    //   注意没加 bias,所以会产生 shadow acne(见文件头警告)。
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

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
    // specular —— 用 Blinn-Phong(halfway 向量)代替 reflect 方向,更省更准
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    // ⭐ 阴影只压掉漫反射+高光,环境光保留(否则阴影区纯黑不自然)。
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
}