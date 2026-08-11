// 物体的片段着色器 — ⭐ 本课核心:用 struct 组织材质和光源
//
// 之前 ambient/diffuse/specular 的"光源强度"和"物体反射率"混在一起写死。
// 本节拆开:
//   struct Material — 物体怎么反光(反射率 + 高光锐度),跟着物体走
//   struct Light    — 光源怎么发光(位置 + 各分量强度),跟着光源走
//
// 新公式(每项都是"光源强度 × 物体反射率"):
//   ambient  = light.ambient  * material.ambient
//   diffuse  = light.diffuse  * (diff * material.diffuse)
//   specular = light.specular * (spec  * material.specular)
//   result   = ambient + diffuse + specular
//
// 直觉:同样的物体(材质固定),换不同色光(光源变),颜色就变;
//       同样的光(光源固定),换不同材质(物体变),反光质感就变。两者解耦。
#version 330 core
out vec4 FragColor;

// 材质结构体——描述"物体表面如何反光",与具体光源无关。
struct Material {
    vec3 ambient;    // 环境光反射率(物体在环境光下显什么色)
    vec3 diffuse;    // 漫反射反射率(物体在直射光下显什么色,通常 = 主色)
    vec3 specular;   // 镜面反射反射率(高光颜色;金属倾向本色,塑料偏白)
    float shininess; // 高光锐度(越大高光越集中、越小)
};

// 光源结构体——描述"光源的属性",与具体物体无关。
struct Light {
    vec3 position;

    vec3 ambient;    // 环境光强度(通常很弱)
    vec3 diffuse;    // 漫反射光强度(主光色)
    vec3 specular;   // 镜面光强度(高光,通常接近白)
};

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform Material material;   // cpp 用 "material.diffuse" 等方式设置成员
uniform Light light;

void main()
{
    // 环境光:光源的环境光强度 × 物体的环境光反射率
    vec3 ambient = light.ambient * material.ambient;

    // 漫反射(同 2.1,但乘上 material.diffuse)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // 镜面反射(同 2.2,但用 material.shininess 和 material.specular)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    // 注意:result 不再乘 objectColor 了——颜色信息已融入 material.diffuse/ambient。
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
