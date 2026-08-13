// 深度遍的顶点着色器 — 只做 model 变换,把顶点送到世界空间
//
// ⚠ 这里【只乘 model】,投影 × lookAt 留到 GS 里【每个面单独应用】。
//   因为同一个三角形要被复制到 6 个面,每个面用的 shadowMatrices[face] 不同,
//   所以 vs 阶段不能决定最终裁剪位置,只能先把世界坐标算出来交给 GS。
//   注意:gl_Position 在这里其实是【世界空间位置】,GS 会再读它并做光空间变换。
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;

void main()
{
    // 这里输出的是世界空间坐标(不是裁剪空间),GS 会进一步处理。
    gl_Position = model * vec4(aPos, 1.0);
}