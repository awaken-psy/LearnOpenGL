// 视差贴图(基础版)的片段着色器 — ⭐ ParallaxMapping 沿视角偏移 UV,假装有深度
//
// 原理一句话:表面"凸起"的地方,你斜着看时,实际看到的纹素应该往视线方向挪一点。
// 近似做法是直接按 height × viewDir.xy 把 UV 反向推出去——height 越大、视角越斜,推得越多。
// 这是个粗暴近似(假设所有高度都在表面上方),所以叫"with offset limiting"。
//
// ⚠ 越界 UV 直接 discard:偏太多就别画了,否则边缘会拉伸出一片鬼影。
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
    // 在原始 UV 处采样高度图,得到当前像素的"凸起量"(0=谷,1=峰)。
    float height =  texture(depthMap, texCoords).r;
    // ⭐ 核心:UV 沿视线水平分量(viewDir.xy)反向偏移 height × heightScale。
    //   直觉:站得越斜、凸起越高,看到的纹素就越该往视线方向挪。
    //   只用 viewDir.xy(不除 z)是 "offset limiting" 简化,能压制极端失真但不够准。
    return texCoords - viewDir.xy * (height * heightScale);
}

void main()
{           
    // offset texture coordinates with Parallax Mapping
    // viewDir 在【切线空间】(由 vs 转好的两个 Tangent 位置相减得到)。
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;

    // ⭐ 用视差函数算出偏移后的 UV,后面所有贴图(法线/漫反射)都用这套新 UV 采样。
    texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);
    // ⚠ 偏移后 UV 出 [0,1] 范围就丢弃该片段,防止边缘采样到重复纹理形成鬼影。
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