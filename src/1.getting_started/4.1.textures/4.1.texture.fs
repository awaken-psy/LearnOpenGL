// 片段着色器
// 新增内容：sampler2D 纹理采样器类型 + texture() 采样函数
#version 330 core

out vec4 FragColor;

// 从顶点着色器传下来的插值后的数据
in vec3 ourColor;
in vec2 TexCoord;

// sampler2D — 纹理采样器类型。
// 它本身不存像素数据，而是"指向某个纹理单元的引用"。
// C++ 端通过 glUniform1i 告诉它去哪个纹理单元（GL_TEXTURE0/1/...）取数据。
uniform sampler2D texture1;

void main()
{
    // texture(sampler, coord) — GLSL 内置函数，从纹理中按坐标采样一个像素颜色。
    //   sampler = 用哪个纹理采样器（texture1）
    //   coord   = 采样坐标（TexCoord，已由 GPU 插值）
    // 返回 vec4（RGBA）。
    FragColor = texture(texture1, TexCoord);
}
