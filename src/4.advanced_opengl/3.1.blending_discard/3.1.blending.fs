// 3.1.blending.fs —— 混合 discard 演示的片段着色器
//
// ⭐ 核心逻辑：采样纹理后检查 alpha 通道，如果几乎完全透明就丢弃该片段。
// discard 的作用是终止片段着色器并丢弃当前片段——
// 不写入颜色缓冲，不写入深度缓冲，就像这个像素从未存在过。
// 这种方法适用于"非全即无"的透明纹理（如草、铁丝网等），
// 但无法处理真正的半透明效果（如玻璃、烟雾）。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{             
    vec4 texColor = texture(texture1, TexCoords);
    // ⭐ alpha < 0.1 的像素被视为完全透明，直接丢弃
    // 被丢弃的片段不会写入颜色/深度缓冲，后方物体不受遮挡
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}