// 10.1 实例化顶点着色器
// 接收 3 个顶点属性：位置、颜色、偏移量
// ⭐ aOffset 是【实例化属性】——每个实例只有一个值，而非每个顶点一个值
// 顶点位置加上实例偏移量，实现 100 个四边形分布在不同位置

#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset; // ⭐ 实例化属性：每个实例的偏移量

out vec3 fColor;

void main()
{
    fColor = aColor;
    // ⭐ 每个实例的顶点位置加上该实例的偏移量，使四边形出现在不同位置
    gl_Position = vec4(aPos + aOffset, 0.0, 1.0);
}
