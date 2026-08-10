// 片段着色器
// 新增内容：两个 sampler2D + mix() 函数把两张纹理按比例混合
#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// 两个纹理采样器，分别绑定到不同的纹理单元（C++ 端 ourShader.setInt 配置）
uniform sampler2D texture1;  // 木板纹理 → 纹理单元 0
uniform sampler2D texture2;  // 笑脸纹理 → 纹理单元 1

void main()
{
    // mix(x, y, a) — GLSL 内置函数，线性插值：结果 = x*(1-a) + y*a
    //   x = texture1 采样结果（木板）
    //   y = texture2 采样结果（笑脸）
    //   a = 0.2 → 20% 取笑脸，80% 取木板
    // 结果：木板上叠着淡淡的笑脸
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
}
