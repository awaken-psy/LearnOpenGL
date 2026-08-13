// 陡峭视差贴图(Steep Parallax Mapping)— 把深度 [0,1] 切成 N 层,沿视线 raymarch 找交点
//
// 5.1 的偏移是一次性的,角度一大就穿帮。陡峭视差改成"切片":
//   把高度 [0,1] 等分成 N 层,从最浅一层开始,沿视线方向(P 向量)一步步往里走,
//   每走一层就比对"当前层深度"和"高度图采样值",第一次发现采样值 < 层深度,
//   就说明视线在这层穿过了表面——取该层的 UV 作为结果。
//
// 结果是一组离散的 UV(层数越多越细,但有锯齿),5.3 的 POM 会再插值把它磨平。
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform float heightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    // number of depth layers
    // 【自适应层数】:正对表面(dot≈1)时用少层(8),掠射角(dot≈0)时用多层(32)。
    //   正对时偏移本就小,少分层也准;掠射时失真大,得多分层补救。性能/质量自动平衡。
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    // ⭐ P = viewDir.xy / viewDir.z × heightScale:【除以 viewDir.z 做透视校正】。
    //   5.1 里只用 viewDir.xy 是粗暴近似;这里除 z 后,远处层间距会自动拉开,
    //   更符合"视线穿过高度场"的真实几何。deltaTexCoords 是每层的 UV 步进。
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    // ⭐【raymarch 主循环】:沿视线逐层步进,直到"当前层深度"首次追上高度图值。
    //   条件 currentLayerDepth < currentDepthMapValue:视线还没钻进表面,就继续走下一层。
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // 走出循环时,视线刚好穿过表面——返回这一层的 UV。
    return currentTexCoords;
}

void main()
{           
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;
    
    texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);       
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    // obtain normal from normal map
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);   
   
    // get diffuse color
    vec3 color = texture(diffuseMap, texCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}