// SSAO Blur 片段着色器 —— 4×4 box 模糊去噪
//
// SSAO pass 输出的遮蔽图有噪声斑点(因为 4×4 噪声纹理平铺造成的重复模式)。
// 这里对每个像素取它周围 4×4 邻域的平均值,抹平噪点,得到平滑的 AO 图。
#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoInput;

void main()
{
    // textureSize() 返回纹理实际像素尺寸,texelSize = 1/尺寸 = 一个像素的 UV 步长。
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    // 遍历当前像素周围 4×4(x∈[-2,2), y∈[-2,2))的邻域,累加 AO 值。
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, TexCoords + offset).r;
        }
    }
    // 除以采样总数(4×4=16)取平均,输出模糊后的 AO。
    FragColor = result / (4.0 * 4.0);
}  