// 屏幕四边形片元着色器 — 后处理入口
// 采样 FBO 颜色纹理并输出到默认帧缓冲（屏幕）
// ⭐ 这是从 FBO 纹理到屏幕的"最后一站"，后处理滤镜效果应在此处添加
//   例如：反色、模糊、灰度等——只需要修改这里的采样逻辑

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
} 
