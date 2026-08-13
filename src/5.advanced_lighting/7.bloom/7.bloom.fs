// 场景光照片段着色器(Bloom 版)— ⭐【MRT 多渲染目标】同时输出"场景色"和"亮部色"
//
// 【MRT(Multiple Render Targets)】:一个 fragment shader 往多个颜色附件写值。
//   layout(location=0) out → GL_COLOR_ATTACHMENT0(FragColor,正常场景)
//   layout(location=1) out → GL_COLOR_ATTACHMENT1(BrightColor,只写亮部 >1.0 的像素)
// 这样一次 draw 就能把"亮到溢出"的像素单独抠出来,作为后面 Bloom 模糊的输入。
// (cpp 里必须配 glDrawBuffers(2, ...) 才能让两个附件都生效。)
//
// 【亮度提取】:亮度 = dot(rgb, vec3(0.2126, 0.7152, 0.0722))。
//   这组系数是【Rec.709】标准 —— 人眼对绿光最敏感(0.7152)、蓝光最不敏感(0.0722),
//   加权算出来的"感知亮度"比取 max(r,g,b) 更符合眼睛实际感受。
//   亮度 >1.0 的像素才算"亮部",写进 BrightColor;否则写黑色(模糊时不污染周围)。
#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Light {
    vec3 Position;
    vec3 Color;
};

uniform Light lights[4];
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
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    for(int i = 0; i < 4; i++)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - fs_in.FragPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 result = lights[i].Color * diff * color;
        // attenuation (use quadratic as we have gamma correction)
        // 物理平方衰减 1/d²(同 6.hdr,配合 gamma 校正)
        float distance = length(fs_in.FragPos - lights[i].Position);
        result *= 1.0 / (distance * distance);
        lighting += result;

    }
    vec3 result = ambient + lighting;
    // check whether result is higher than some threshold, if so, output as bloom threshold color
    // ⭐ 用 Rec.709 系数算感知亮度。dot(·) 就是加权求和:r·0.2126 + g·0.7152 + b·0.0722。
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);              // 亮部:写原色,交给后面 blur
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);       // 不亮:写黑,模糊时不影响周围
    FragColor = vec4(result, 1.0);
}
