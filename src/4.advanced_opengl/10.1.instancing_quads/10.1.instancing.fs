// 10.1 实例化片段着色器
// 直接输出从顶点着色器传递过来的颜色

#version 330 core
out vec4 FragColor;

in vec3 fColor;

void main()
{
    FragColor = vec4(fColor, 1.0);
}
