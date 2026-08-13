// Lighting Pass 片段着色器 —— ⭐ 8.2 相对 8.1 的唯一差异:【光体积剔除】
//
// 主体逻辑(从 G-Buffer 采样、循环 32 光源、Blinn-Phong + 衰减)与 8.1.deferred_shading.fs 相同。
// 新增:Light 结构体多了 Radius 字段,for 循环里用 if(distance < Radius) 提前剔除
// 影响不到当前 fragment 的光源 —— 距离太远的光直接跳过,省掉漫反射/高光/衰减一整套计算。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
    float Radius;   // ⭐ 新增:光体积半径(C++ 用求根公式算出,见 cpp)
};
const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main()
{
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // ⭐ 先算光源到当前 fragment 的距离,超出 Radius 直接跳过整个光源的计算。
        //   这就是"光体积剔除":每个光源只影响以其为中心、半径 Radius 的球内的 fragment。
        // calculate distance between light source and current fragment
        float distance = length(lights[i].Position - FragPos);
        if(distance < lights[i].Radius)
        {
            // diffuse
            vec3 lightDir = normalize(lights[i].Position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].Color * spec * Specular;
            // attenuation
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }
    FragColor = vec4(lighting, 1.0);
}
