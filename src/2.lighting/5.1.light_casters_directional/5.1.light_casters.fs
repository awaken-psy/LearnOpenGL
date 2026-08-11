// 物体的片段着色器 — ⭐ 方向光(太阳):只有方向,无位置,无衰减
//
// 关键改动(相对 4.2):
//   - struct Light 用 direction(光的方向)替代 position
//   - lightDir = normalize(-light.direction):direction 是"光射向哪里",取反得到"片段指向光源"
//   - 没有衰减(方向光在无限远,距离不影响亮度)
//
// 为什么 lightDir 要取反?
//   direction (-0.2,-1,-0.3) 表示光"朝这个方向射"(向下)。
//   而漫反射/镜面反射公式里的 lightDir 需要"从片段指向光源"(向上),方向相反,所以加负号。
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    //vec3 position;   // 方向光不需要位置(注释掉)
    vec3 direction;   // 光的方向("光射向哪里")

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
    // 环境光
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    // 漫反射
    vec3 norm = normalize(Normal);
    // ⭐ 方向光:lightDir = -direction(对所有片段都一样,因为光线平行)。
    //   对比点光源的 lightDir = normalize(position - FragPos)(每个片段不同)。
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    // 镜面反射(同前)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    // 注意:没有衰减项——方向光在无限远,距离不影响亮度。
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
