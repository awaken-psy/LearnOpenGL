// 天空盒顶点着色器 — 利用深度技巧让天空盒始终在最远处
//
// ⭐ 核心技巧：gl_Position = pos.xyww
//   将裁剪空间 z 分量设为 w（而非 z），经透视除法后 z/w = 1.0，
//   即 NDC 中 z 始终为 1.0（最远平面）。
//   配合 C++ 端的 glDepthFunc(GL_LEQUAL)，天空盒就能通过深度测试被绘制。
//
// ⭐ 顶点位置直接作为 TexCoords 传递给片元着色器：
//   立方体贴图用方向向量采样，顶点位置恰好就是从原点出发的方向

#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}	
