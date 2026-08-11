// 场景顶点着色器 — 渲染立方体和地板到 FBO
// 与普通 3D 场景着色器完全相同：接收位置+纹理坐标，输出经过 MVP 变换的裁剪空间位置
// 这里渲染的结果会被写入帧缓冲的颜色附件纹理，而非直接显示在屏幕上

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
