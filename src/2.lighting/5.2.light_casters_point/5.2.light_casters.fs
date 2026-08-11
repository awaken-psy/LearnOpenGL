// 物体的片段着色器 — ⭐ 点光源:有位置 + 距离衰减
//
// 新增内容(相对 4.2):
//   - struct Light 加 constant/linear/quadratic 三个衰减系数
//   - 衰减公式:attenuation = 1 / (constant + linear*d + quadratic*d²),d = 片段到光源距离
//   - 三项光照(ambient/diffuse/specular)都乘 attenuation → 距离越远整体越暗
//
// 直觉:灯泡照不亮远处的墙。物理上光强按距离平方衰减(quadratic),
//       但纯二次衰减近处太亮、远处太快变黑,加 linear 和 constant 项是为了"看起来更自然"。
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // 新增:衰减系数
    float constant;   // 常数项(通常 = 1)
    float linear;     // 线性项
    float quadratic;  // 二次项
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    // 环境光 + 漫反射 + 镜面反射:同 4.2(用 light.position 算 lightDir)
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    // ⭐ 衰减:距离越远,attenuation 越小(趋近 0),光越暗。
    //   length() 求向量长度 = 片段到光源的距离 d。
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // 三项光照都乘衰减(整体随距离变暗)
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
