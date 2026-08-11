/**
 * 练习 4.3 — 反相 specular 贴图(让"该亮的不亮、该暗的亮")
 *
 * 把 specular 贴图采样值取反:specular = ... * (vec3(1.0) - texture(specular))。
 * 效果:原本反光的铁边框变哑光,原本哑光的木板反而反光——一种"负片"高光效果。
 *
 * ⚠ 下方是【片段着色器】GLSL 源码(不是 C++),用 #if 0 屏蔽,仅供阅读。
 *    要运行:把这段 fs 替换 4.2 的 4.2.lighting_maps.fs 即可(vs 不变)。
 *
 * 核心改动(相对 4.2.fs):
 *   vec3(texture(...)) 用 vec3() 显式构造(和 .rgb 等价);
 *   specular 项乘 (1 - 采样值),让黑白颠倒。
 */

#if 0
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
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
    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // ⭐ 取反:用 (1 - 采样值),黑变白、白变黑,反相高光区域
    vec3 specular = light.specular * spec * (vec3(1.0) - vec3(texture(material.specular, TexCoords)));

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
#endif
