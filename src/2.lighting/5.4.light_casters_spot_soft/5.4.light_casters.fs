// 物体的片段着色器 — ⭐ 聚光灯(软边):内外锥之间平滑过渡
//
// 5.3 的硬边聚光灯锥边有锯齿(if/else 突变)。本节改成软边:
// 多一个 outerCutOff(外锥角,比内锥大),在内锥与外锥之间让亮度平滑地从 1 降到 0。
//
// 核心:intensity = clamp((theta - outerCutOff) / (cutOff - outerCutOff), 0, 1)
//   theta 在 [outerCutOff, cutOff] 之间时,intensity 从 0 平滑到 1;
//   超出外锥 → 0;在内锥内 → 1。用乘 intensity 代替 if/else,边缘自然柔和。
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;       // 内锥角余弦(锥内全亮)
    float outerCutOff;  // 外锥角余弦(比内锥大;内外锥之间渐变到 0)

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    // 环境光 + 漫反射 + 镜面反射:先按"全亮"算,后面再用 intensity 收口
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    // ⭐ 软边:在内外锥之间平滑过渡(代替 5.3 的 if/else 硬切)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.cutOff - light.outerCutOff);            // 内外锥的余弦差
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    //   theta >= cutOff(内锥内):intensity = 1,全亮
    //   theta <= outerCutOff(外锥外):intensity = 0,不亮
    //   之间:intensity 从 0→1 平滑过渡 → 边缘柔和
    diffuse  *= intensity;
    specular *= intensity;   // 只压漫反射和镜面;环境光保留(锥外也微亮)

    // 衰减(同 5.2)
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
