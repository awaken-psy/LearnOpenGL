// 1.1.depth_testing.vs —— 深度测试演示的顶点着色器
// 标准的顶点变换：将顶点位置通过 MVP 矩阵变换到裁剪空间，
// 并将纹理坐标传递给片段着色器。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}