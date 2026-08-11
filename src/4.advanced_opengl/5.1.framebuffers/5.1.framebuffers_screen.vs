// 屏幕四边形顶点着色器 — 将 FBO 纹理贴到全屏四边形上
// ⭐ 关键点：不使用 MVP 矩阵，因为顶点坐标已经是 NDC（[-1,1] 范围）
//   直接将 xy 坐标作为裁剪空间坐标，z 设为 0.0
//   这样四边形会铺满整个屏幕，无需任何矩阵变换

#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}	
