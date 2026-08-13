// Bloom 最终合成片段着色器 — ⭐ 场景色 + 模糊亮部 = 泛光,再 tonemap + gamma
//
// 两张纹理输入:
//   scene      = Pass 1 的正常 HDR 场景(colorBuffers[0])
//   bloomBlur  = Pass 2 高斯模糊后的亮部(pingpongColorbuffers)
//
// 合成方式:hdrColor += bloomColor【加性混合】—— 亮部直接加到原场景上,形成"溢出的光晕"。
// 之后流程同 6.hdr.fs:exposure tonemap 把 [0,∞) 压回 [0,1] → gamma 校正 → 输出到屏幕。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;            // = false 时跳过加性混合(只输出场景,对比看 Bloom 效果)
uniform float exposure;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if(bloom)
        // ⭐【加性混合】把模糊的亮部直接加到场景上 —— 这是 Bloom 的核心一步。
        //   亮的更亮、并向周围暗区溢出,形成柔和光晕。
        hdrColor += bloomColor; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}