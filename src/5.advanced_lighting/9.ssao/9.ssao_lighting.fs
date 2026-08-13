// SSAO Lighting Pass 片段着色器 —— 延迟 Blinn-Phong,ambient 项乘上 AO 遮蔽因子
//
// 与 8.1.deferred_shading.fs 的差异:
//   - ambient 不再是固定 0.1,而是 0.3 * Diffuse * AmbientOcclusion(从 SSAO 纹理采样)
//   - 单光源(不是 32 个数组),简单 Blinn-Phong
//   - view space 下相机在原点,viewDir = normalize(-FragPos)(不用传 viewPos)
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;   // 模糊后的 SSAO 遮蔽纹理

struct Light {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
};
uniform Light light;

void main()
{
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;

    // ⭐【SSAO 的核心应用】ambient 项乘上 AO 因子:角落处 AO→0,环境光变暗;
    //   开阔处 AO→1,环境光正常。这就是 SSAO 带来的接触阴影效果。
    // then calculate lighting as usual
    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
    vec3 lighting  = ambient;
    // ⚠ view space 下相机在原点(0,0,0),所以【视线方向 = 原点 - FragPos = -FragPos】。
    //   不需要像 8.1 那样传 viewPos uniform。
    vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0)
    // diffuse
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.Color * spec;
    // attenuation
    float distance = length(light.Position - FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    FragColor = vec4(lighting, 1.0);
}
