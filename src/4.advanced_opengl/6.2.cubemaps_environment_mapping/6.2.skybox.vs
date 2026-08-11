// 天空盒顶点着色器 — 与 6.1 完全相同
//
// ⭐ 核心技巧：gl_Position = pos.xyww
//   将 z 分量替换为 w，使深度值始终为 1.0（最远平面）
//   配合 GL_LEQUAL 深度函数使天空盒通过深度测试
//
// 顶点位置直接作为方向向量传递给片元着色器采样立方体贴图

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
