// 地板片段着色器 —— ⭐【Gamma 校正 / sRGB 线性工作流】+ 物理正确衰减
//
// 三件事(对应 cpp 三步):
//   1. 输入:floorTexture 由 cpp 用 GL_SRGB 内部格式加载 → texture() 返回【线性】光强(自动解码)
//   2. 计算:BlinnPhong() 在线性空间做光照,衰减用【1/d²】(gamma 开时)—— 物理正确的距离平方反比
//   3. 输出:gamma 开时,最终颜色 pow(color, 1/2.2) 编码回 sRGB,交给显示器
//
// 对比:gamma 关时退回 1/d 衰减 + 不做输出校正(老办法,看着「还行」其实不物理)。
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D floorTexture;

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 viewPos;
uniform bool gamma;

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
    // diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // 【距离衰减 attenuation】物理正确:光强按距离平方反比衰减 I ∝ 1/d²。
    //   gamma 开 → 1/d²(真实物理,本节推荐)
    //   gamma 关 → 1/d (经验近似;因显示器把暗部压暗,1/d 看着反而「正常」)
    // ⚠ 这就是为什么开 gamma 必须同时改用 1/d²,否则衰减太陡、远处全黑。
    // simple attenuation
    float max_distance = 1.5;   // ⚠ 原版遗留,声明后未参与计算(保留不动)
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (gamma ? distance * distance : distance);
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return diffuse + specular;
}

void main()
{
    // 采样纹理:若 cpp 绑定的是 sRGB(GL_SRGB)版本,GPU 在这里自动返回线性值;否则返回原始 sRGB 值。
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
    vec3 lighting = vec3(0.0);
    // 4 个光源叠加:BlinnPhong 返回每个光的漫反射+高光(已含衰减),累加得到总光照(线性空间)。
    for(int i = 0; i < 4; ++i)
        lighting += BlinnPhong(normalize(fs_in.Normal), fs_in.FragPos, lightPositions[i], lightColors[i]);
    // 线性纹理色 × 线性光照 = 线性结果,准备好送去 gamma 编码。
    color *= lighting;
    // ⭐ 输出端 gamma 校正:把线性颜色 pow(color, 1/2.2) 编码回 sRGB 交给显示器。
    //   显示器本身会再做一次 pow(·, 2.2) 发光,两次幂运算抵消,人眼最终看到的是正确的线性亮度。
    //   vec3(1.0/2.2):对 R/G/B 三个通道各取 1/2.2 次方。
    if(gamma)
        color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, 1.0);
}