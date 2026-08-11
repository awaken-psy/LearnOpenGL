// 片段着色器：几何着色器房屋示例
// 直接输出几何着色器传递过来的颜色（墙壁用顶点颜色，屋顶为白色）。
#version 330 core
out vec4 FragColor;

in vec3 fColor;

void main()
{
    FragColor = vec4(fColor, 1.0);   
}