// 11.2 后处理屏幕四边形片段着色器
// ⭐ 采样已解析的纹理（多采样 → 单采样后的结果），并应用灰度后处理效果
// 后处理演示了为什么要用离屏 MSAA：需要先解析多采样缓冲，才能在后处理中采样

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    // 采样屏幕纹理（已是抗锯齿后的单采样纹理）
    vec3 col = texture(screenTexture, TexCoords).rgb;
    // ⭐ 后处理示例：将颜色转换为灰度图
    // 使用感知亮度加权公式（人眼对绿色更敏感）
    float grayscale = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
    FragColor = vec4(vec3(grayscale), 1.0);
}  
