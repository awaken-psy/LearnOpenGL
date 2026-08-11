// 3.2.blending.fs —— Alpha 混合演示的片段着色器
//
// ⭐ 与 3.1 的关键区别：这里不使用 discard，而是直接输出包含 alpha 通道的纹理颜色。
// 混合操作由 OpenGL 固定管线在片段着色器之后自动完成：
//   最终颜色 = 源颜色 × alpha + 目标颜色 × (1 - alpha)
// 因为启用了 GL_BLEND 并设置了 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)，
// 窗口纹理中的半透明区域会与后方已有颜色正确混合。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{             
    FragColor = texture(texture1, TexCoords);
}