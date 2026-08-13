// 主片段着色器 — ⭐ 用【方向向量采样 samplerCube】算点光源阴影
//
// 与 3.1 的 2D 阴影相比,核心区别有三处:
//   ① 采样器:samplerCube(不是 sampler2D),采样用【vec3 方向向量】,不是 (u,v)。
//   ② 采样方向:用【从光源指向片段的方向向量】(fragPos - lightPos,变量名 fragToLight)
//     作为 cubemap 的采样方向,GPU 自动选中对应的面 + 纹素位置(就像天空盒那样)。
//   ③ 深度含义:depth cubemap 存的是【线性距离 / far_plane】(depth.fs 手写的),
//     不是 2D 阴影里硬件自动写的非线性深度。所以读出后要 ×far_plane 还原成真实距离,
//     bias 也要比 2D 阴影大一些(见下)。
//
// 光照部分是 Blinn-Phong(ambient + diffuse + specular),已在 2.x 学过,不再重复注释。
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
// ⭐ samplerCube:立方体深度贴图。采样时用 vec3 方向向量(不是 vec2 的 uv)。
//   和第4章天空盒的 cubemap 用法相同,只是里面装的是【深度值】不是颜色。
uniform samplerCube depthMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;

float ShadowCalculation(vec3 fragPos)
{
    // 【采样方向向量】= fragPos - lightPos,即从光源指向片段的方向。
    //   cubemap 采样看的是方向:这个向量指向 cubemap 的某个面 + 某个纹素,
    //   取回的值就是该方向上【离光源最近的不透明物】到光源的距离(已归一化)。
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // ⭐ 用方向向量采样 cubemap。texture(samplerCube, vec3) 返回该方向的纹素值。
    //   .r 取红色通道(depth cubemap 只有一个通道,存在 r 里)。
    // ise the fragment to light vector to sample from the depth map
    float closestDepth = texture(depthMap, fragToLight).r;
    // 【还原】:depth cubemap 存的是 [0,1] 归一化距离(在 depth.fs 里除过 far_plane),
    //   乘 far_plane 还原成真实世界距离,才能和下面的 currentDepth 比。
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // 当前片段到光源的真实 3D 距离(欧氏长度)。和 fragToLight 是同一个向量,length 相同。
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // ⚠ bias 比 2D 阴影大:因为存的是真实距离(范围 0~far_plane=25),
    //   不是 [0,1] 的非线性深度,精度分布不同,偏移量要给大些避免阴影瑕疵。
    // test for shadows
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // 当前片段比最近深度更远 → 光被挡住了 → 在阴影里(返回 1.0)。
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

    return shadow;
}

void main()
{
    // 下面是标准 Blinn-Phong 光照(2.x 已学),不再逐行注释。
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
    // ⭐ 算阴影(shadows 是 cpp 端 SPACE 切换的开关)。在阴影里 → 只保留 ambient。
    // calculate shadow
    float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
}