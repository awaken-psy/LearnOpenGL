/**
 * 练习3：用顶点位置作为颜色输出
 *
 * 和 3.2 的区别：3.2 每个顶点自带颜色（aColor），这里改用顶点坐标（aPos）当颜色。
 * 片段着色器输出的颜色 = 每个像素在三角形内部的位置坐标（插值后）。
 *
 * 为什么左下角是黑色的？
 * — 左下顶点的坐标是 (-0.5, -0.5, 0.0)，xy 都是负数。
 *   GLSL 中颜色分量会被截断（clamp）到 [0.0, 1.0] 范围，负数变为 0.0。
 *   从边角到三角形中心，xy 逐渐从负变正 → 颜色从黑渐变到彩色。
 *   值 0.0 就是黑色，所以左下角到中心之间都是暗的。
 */

// 顶点着色器
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

// out vec3 ourColor;
out vec3 ourPosition;       // 输出顶点位置而非颜色

void main()
{
    gl_Position = vec4(aPos, 1.0);
    // ourColor = aColor;
    ourPosition = aPos;     // 把顶点位置传给片段着色器
}

// 片段着色器
#version 330 core
out vec4 FragColor;
// in vec3 ourColor;
in vec3 ourPosition;        // 接收插值后的位置

void main()
{
    // 把位置坐标直接当 RGB 颜色使用
    // 左下顶点 (-0.5, -0.5) → 负值截断为 0 → 黑色
    // 顶部顶点 (0.0, 0.5)   → (0, 0.5, 0) → 绿色
    // 中心附近              → x≈0, y≈0 → 接近黑
    FragColor = vec4(ourPosition, 1.0);
}
