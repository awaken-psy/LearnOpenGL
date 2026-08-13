// Lighting Pass 片段着色器 — ⭐ 延迟渲染的核心:从 G-Buffer 采样几何,循环 32 个光源算光照
//
// 这里的"片段"是全屏 quad 上的一个像素,不是 3D 模型的顶点插值片段。
// 通过 TexCoords(0~1 屏幕坐标)从 G-Buffer 采样,把该像素对应的几何信息"读回来",
// 然后用标准 Blinn-Phong + 衰减逐光源累加 —— 和前向渲染光照公式完全一样,只是数据来源变了。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// 3 张 G-Buffer 纹理(cpp 里绑到 texture unit 0/1/2,这里采样)
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

// 光源结构体:位置、颜色、衰减系数(线性项 + 二次项)。常数项隐含为 1.0。
struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
};
const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main()
{
    // ⭐ 从 G-Buffer 采样回几何信息 —— 延迟渲染的关键:数据来自纹理,不是顶点插值。
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // 环境光硬编码 0.1(后面 SSAO 章节会把这部分替换成屏幕空间遮蔽)。
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    // ⭐ 32 个光源逐个累加 —— 延迟渲染下这只是 32 次 ALU 循环,几何早已存好,开销可控。
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
        // specular(Blinn-Phong 用 halfway 半角向量,第 2 章已学)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = lights[i].Color * spec * Specular;
        // attenuation 衰减:1 / (常数 + 线性·d + 二次·d²),距离越远光越弱。
        float distance = length(lights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
    FragColor = vec4(lighting, 1.0);
}
