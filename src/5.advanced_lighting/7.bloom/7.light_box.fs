// 光源立方体片段着色器 — 直接输出光色,也走【MRT】把亮部写进 BrightColor
//
// 双输出结构和 7.bloom.fs 完全一致(location 0 = FragColor,location 1 = BrightColor),
// 但内容更简单:不计算光照,直接把 uniform lightColor 作为颜色输出(光源本身发光)。
// 亮度提取逻辑也相同 —— Rec.709 系数 dot 后 >1.0 才算亮部。
//
// ⚠ 这 4 个 cube 光源必须也参与 Bloom 的亮部提取。否则它们在亮部图(colorBuffers[1])里
//    是黑的,模糊后没有光晕,最终合成时就看不到光源"溢出"的效果。
#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform vec3 lightColor;

void main()
{
    FragColor = vec4(lightColor, 1.0);
    // 亮度提取(Rec.709 系数),逻辑同 7.bloom.fs
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}