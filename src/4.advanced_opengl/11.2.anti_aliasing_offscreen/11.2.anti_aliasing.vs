// 11.2 离屏 MSAA 场景顶点着色器
// 标准的 MVP 变换，用于将立方体渲染到多采样帧缓冲中

#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
