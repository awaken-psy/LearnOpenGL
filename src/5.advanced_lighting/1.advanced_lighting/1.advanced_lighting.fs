// 地板片段着色器 —— ⭐ 对比【Phong】vs【Blinn-Phong】两种高光模型
//
// Phong 高光(第2章):R = reflect(-L, N);spec = pow(max(dot(V, R), 0), 8)
// Blinn-Phong 改用【半程向量】H = normalize(L + V);spec = pow(max(dot(N, H), 0), 32)
//   H 是「光与视线夹角的中分线」方向。N 越接近 H → 高光越强。
//
// 为什么指数从 8 跳到 32?因为 N 与 H 的夹角约为 V 与 R 夹角的一半,余弦曲线更平,
// 必须用更大的 shininess 才能把高光收窄到相近大小。按 B 运行时切换对比。
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D floorTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool blinn;

void main()
{           
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.05 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    // ⚠ 下面这行 reflectDir 在 if 里没被用到:Blinn 分支用 H;Phong 分支在 else 内重新声明了一份。
    //   属于原版遗留代码,保留不动。
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        // ⭐【Blinn-Phong】半程向量 H = normalize(L + V)。
        //   直觉:H 指向「光和眼睛方向的中间」,N·H 越大表示表面朝向越接近这个中间方向。
        vec3 halfwayDir = normalize(lightDir + viewDir);
        // 高光 = (N·H)^32。指数 32 远大于 Phong 的 8 —— 因 N·H 夹角约为 V·R 的一半。
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        // 【Phong】(第2章原版,做对照):用反射向量 R = reflect(-L, N) 和视线 V 比夹角。
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    vec3 specular = vec3(0.3) * spec; // assuming bright white light color
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}