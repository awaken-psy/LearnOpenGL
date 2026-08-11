// 物体的片段着色器 — ⭐ 本课核心:镜面反射贴图
//
// 在 4.1 基础上,把 specular 也从单色改成贴图。
// specular map 是黑白图:白 = 反光、黑 = 不反光,让物体"部分反光"(如木箱铁框反光、木板哑光)。
//
// 关键改动(相对 4.1):
//   struct Material { sampler2D diffuse; sampler2D specular; ... }   ← specular 也变采样器
//   specular = light.specular * spec * texture(material.specular, TexCoords).rgb
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;    // 漫反射贴图(表面颜色)
    sampler2D specular;   // ⭐ 镜面反射贴图(黑白:白 = 该处反光,黑 = 哑光)
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    // 环境光 + 漫反射:同 4.1,都从 diffuse 贴图取色
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    // 镜面反射:高光强度从【specular 贴图】采样。
    //   白色像素处 texture().rgb ≈ (1,1,1) → 正常高光;
    //   黑色像素处 ≈ (0,0,0) → 高光被乘 0 → 不反光。
    //   于是只有 specular map 的白色区域(铁边框)出现高光,木板保持哑光。
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;
    // vec3 specular = light.specular * spec * (vec3(1.0) - vec3(texture(material.specular, TexCoords)));

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
