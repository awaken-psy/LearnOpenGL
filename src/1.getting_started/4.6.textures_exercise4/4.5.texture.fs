// 练习4片段着色器
// 新增：uniform float mixValue — 混合比例由 C++ 每帧传入（方向键控制）
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// 混合比例 [0,1]，C++ 端 ourShader.setFloat("mixValue", mixValue) 每帧更新
uniform float mixValue;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    // mix 的第三个参数从硬编码的 0.2 换成了 uniform mixValue
    // → 按上下方向键能实时看到两张纹理的混合比例变化
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixValue);
}
