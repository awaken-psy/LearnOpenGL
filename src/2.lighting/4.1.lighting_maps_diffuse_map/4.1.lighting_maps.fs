// 物体的片段着色器 — ⭐ 本课核心:漫反射贴图
//
// 上一节 material.diffuse 是单色 vec3,本节改成 sampler2D(纹理),
// 物体表面颜色由【漫反射贴图】决定——每个片段按纹理坐标从图里查自己的颜色。
//
// 关键改动:
//   struct Material { sampler2D diffuse; ... }   ← diffuse 从 vec3 变成纹理采样器
//   采样:texture(material.diffuse, TexCoords).rgb   ← 取纹理上对应位置的颜色
//
// ambient/diffuse 现在都从纹理取色;specular 暂时还是单色 vec3(下一节 4.2 改成贴图)。
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;   // ⭐ 漫反射贴图(取代单色)。sampler2D 是"纹理采样器",
                         //    不能直接用,要通过 texture(采样器, 坐标) 取色。
    vec3 specular;       // 镜面反射率(暂时仍是单色,4.2 改成贴图)
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
in vec2 TexCoords;   // 新增:来自 vs 的纹理坐标(已插值)

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    // 环境光:光源环境光强度 × 纹理上该处的颜色
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    // 漫反射:同 2.1 的点乘,但物体颜色从纹理采样
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    // 镜面反射:specular 还是单色 material.specular(下一节改贴图)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
