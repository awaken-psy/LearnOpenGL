// 物体的片段着色器 — ⭐ 聚光灯(硬边):手电筒,锥内亮、锥外暗
//
// 新增内容(相对 5.2):
//   - struct Light 加 direction(朝向)和 cutOff(锥角余弦阈值)
//   - theta = dot(lightDir, -direction):片段到光源方向 与 光轴 的夹角余弦
//   - if theta > cutOff:在锥内 → 正常光照;else:只给 ambient(锥外几乎全黑)
//
// 为什么比较余弦而不是角度?
//   点乘天然给出余弦,直接拿余弦和 cutOff(也是余弦)比较,省去 acos 反三角运算。
//   余弦是减函数:角度越小(越接近光轴)余弦越大,所以"在锥内"判据是 theta > cutOff。
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 direction;  // 聚光灯朝向(光轴方向)
    float cutOff;    // 内锥角的余弦阈值(锥的"半径")

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;   // 衰减三系数(同 5.2)
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
    vec3 lightDir = normalize(light.position - FragPos);

    // theta = 片段到光源方向 与 光轴朝外 的夹角余弦。
    //   -light.direction:direction 是"光射向哪里",取反 = "从光源沿光轴向外看"。
    //   theta 越大(接近 1)→ 片段越在光轴正前方 → 越亮。
    float theta = dot(lightDir, normalize(-light.direction));

    if(theta > light.cutOff) // 在锥内(余弦比较:theta 越大 = 角度越小 = 越靠近光轴)
    {
        // 环境光 + 漫反射 + 镜面反射:同 5.2
        vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

        // 衰减(同 5.2)
        float distance    = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        // ambient 不乘衰减:否则远处锥内反而比锥外(只有 ambient)还暗,不合理。
        
        diffuse  *= attenuation;
        specular *= attenuation;

        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result, 1.0);
    }
    else
    {
        // 锥外:只给一点环境光,避免完全漆黑(也保证锥内外过渡有底色)。
        FragColor = vec4(light.ambient * texture(material.diffuse, TexCoords).rgb, 1.0);
    }
}
