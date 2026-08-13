// 阴影映射(1/3)— 深度图顶点着色器:把顶点变换到【光源空间】
//
// 和普通 vs 的区别:没有 projection×view,而是用 lightSpaceMatrix(=正交投影×lookAt光源)。
// 画深度图只需要顶点位置来算 z,所以不传法线/UV/颜色。
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;   // 【光源空间矩阵】= lightProjection × lightView
uniform mat4 model;              // 物体的世界变换

void main()
{
    // ⭐ gl_Position 直接 = lightSpaceMatrix × model × 顶点。
    //   光栅化后,硬件按这个 z 值做深度测试并写深度图,越近 z 越小。
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}