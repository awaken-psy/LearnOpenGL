// 阴影映射(1/3)— 调试用全屏四边形顶点着色器
//
// quad 的顶点已经在【NDC(-1..1)】里铺满整个屏幕,所以直接当 gl_Position 输出,
// 跳过 MVP。唯一任务是把 UV 传给 fs,用来采样 depthMap 纹理。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    // ⭐ 顶点已是 NDC,直接当裁剪空间坐标用,不需要任何矩阵变换。
    gl_Position = vec4(aPos, 1.0);
}