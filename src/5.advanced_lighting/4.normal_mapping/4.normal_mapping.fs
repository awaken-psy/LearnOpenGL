// 法线贴图的片段着色器 — ⭐ 从法线贴图采样并【解码】,然后在切线空间做 Blinn-Phong
//
// 法线贴图纹素存的是 [0,1] 的 RGB,对应法线的 [-1,1] 三个分量,所以采样后要 *2-1 解码。
// 解出来的法线就是切线空间下的;vs 已经把 lightPos/viewPos/fragPos 也搬到了切线空间,
// 后面的光照算法和以前完全一样,只是所有向量都活在同一个空间里了。
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

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{           
     // obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    // ⭐【解码】纹理存的 [0,1] → 法线的 [-1,1]:每个分量 *2-1。
    //   解出来的就是【切线空间】下的法线,直接用于下面的光照点乘。
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
   
    // get diffuse color
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse(光源、片段位置都用 Tangent* 版本——和 normal 同处切线空间)
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular(viewDir 同样基于切线空间位置)
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}