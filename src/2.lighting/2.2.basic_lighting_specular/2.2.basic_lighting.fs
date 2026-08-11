// 物体的片段着色器 — ⭐ 在 2.1 基础上加【镜面反射 specular】
//
// Phong 完整公式:颜色 = (环境光 + 漫反射 + 镜面反射) × 物体颜色
//
// 镜面反射直觉:光打在表面,按"入射角 = 反射角"弹开。当反射光正好对准你的眼睛,
// 你就看到高光。所以需要:入射光的反射方向 R,和"片段→眼睛"方向 V,两者越接近高光越强。
//
//          眼睛 V (viewDir)
//           ↑   ╱ R (reflectDir,反射方向)
//           │  ╱
//      ─────┼─╱─────── 表面
//        法线 N
//         ↑
//         入射光 L 从这侧来(指向光源),反射后沿 R 弹出
//
// 幂次 32 = shininess:把 (V·R) 取 32 次方,把高光"收窄"成小亮斑。值越大,高光越小越锐。
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;   // 新增:相机位置(算视线方向用)
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // ---- 环境光(同 2.1)----
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // ---- 漫反射(同 2.1)----
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // ---- 镜面反射 specular(本节新增)----
    // specularStrength:高光强度系数,比漫反射小(0.5),因为高光只是个亮点,不该盖过整体。
    float specularStrength = 1.0;
    // viewDir:从片段指向相机(眼睛)的方向。
    vec3 viewDir = normalize(viewPos - FragPos);
    // reflect(I, N):求入射光 I 沿法线 N 的反射方向。
    //   ⚠ reflect 的第一个参数要求是【指向表面的入射方向】(即从光源指向片段),
    //     而 lightDir 是"片段→光源",方向相反,所以这里取 -lightDir。
    vec3 reflectDir = reflect(-lightDir, norm);
    // ⭐ 高光强度 = (视线与反射方向的夹角余弦)^32。
    //   max(·,0):背光面不算高光。
    //   pow(..., 32):把余弦值"锐化",只有夹角很小时才亮,形成小亮斑(32 即 shininess)。
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
    vec3 specular = specularStrength * spec * lightColor;

    // Phong 完整公式:三项相加,再乘物体反射率
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
