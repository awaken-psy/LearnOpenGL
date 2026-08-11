// 物体的片段着色器 — 相对 4.2 新增【自发光 emission 贴图】
//
// emission = 物体自己发的光,与外部光源无关。即使在暗面(光照不到的地方)也会亮,
// 常用于屏幕、岩浆、发光符文等。这里用 matrix.jpg 做发光图案。
//
// 公式变成:result = ambient + diffuse + specular + emission
// emission 直接加进去,不参与光照计算(不受法线/光源影响)。
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;   // 新增:自发光贴图
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
    // ambient + diffuse + specular 同 4.2
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    // 新增:自发光——直接采样 emission 贴图,不乘任何光照系数。
    vec3 emission = texture(material.emission, TexCoords).rgb;

    // 四项相加(emission 让暗面也有光)
    vec3 result = ambient + diffuse + specular + emission;
    FragColor = vec4(result, 1.0);
}
