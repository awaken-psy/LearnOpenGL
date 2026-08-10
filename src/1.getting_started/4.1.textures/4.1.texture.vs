// 顶点着色器
// 新增内容：接收纹理坐标 aTexCoord（vec2），原样传给片段着色器
#version 330 core

// 三个输入属性，分别对应 C++ 端 glVertexAttribPointer(0/1/2, ...) 的数据
layout (location = 0) in vec3 aPos;      // 位置（x, y, z）
layout (location = 1) in vec3 aColor;    // 颜色（R, G, B）
layout (location = 2) in vec2 aTexCoord; // 纹理坐标（u, v）—— vec2 = 2 个 float 的向量

// 输出给片段着色器（名字要和 .fs 里的 in 一致）
out vec3 ourColor;
out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
    // 纹理坐标直接往下传，GPU 会在三角形内部自动插值
    // vec2(aTexCoord.x, aTexCoord.y) 等价于直接写 aTexCoord，这里只是显式构造
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
