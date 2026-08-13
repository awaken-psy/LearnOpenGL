// 场景光照片段着色器 — 16 个光源 + 物理平方衰减,输出到【浮点 FBO】
//
// 和前面 Blinn-Phong 的区别:这里只算漫反射(无 specular),且用纯 1/distance² 物理衰减。
// 平方衰减在【gamma 校正】下视觉上才对;线性衰减在 gamma 下会偏暗(第 5 章学过)。
//
// 输出颜色可能远超 1.0(主光源 200×200×200 × 漫反射 × 衰减后,近处仍可能 >1),
// 所以必须渲染到浮点 FBO,而不是默认的 8 位帧缓冲(否则 >1 被截断)。
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Light {
    vec3 Position;
    vec3 Color;
};

uniform Light lights[16];      // ⭐ 16 盏灯的数组,每盏独立 Position+Color(多光源)
uniform sampler2D diffuseTexture;
uniform vec3 viewPos;

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    // ambient
    vec3 ambient = 0.0 * color;
    // lighting
    vec3 lighting = vec3(0.0);
    for(int i = 0; i < 16; i++)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - fs_in.FragPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = lights[i].Color * diff * color;
        vec3 result = diffuse;
        // attenuation (use quadratic as we have gamma correction)
        // ⭐【物理平方衰减】1/d²。距离翻倍 → 亮度变 1/4。
        //   这里不加常数项(没有 1.0 + a·d + b·d²),是纯物理衰减 ——
        //   所以光源颜色必须设很大(vec3(200,200,200))才在合理距离上够亮。
        float distance = length(fs_in.FragPos - lights[i].Position);
        result *= 1.0 / (distance * distance);
        lighting += result;

    }
    FragColor = vec4(ambient + lighting, 1.0);
}