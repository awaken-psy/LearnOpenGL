// 顶点着色器：UBO 示例
// 本着色器从【UBO】中读取 projection 和 view 矩阵，而非通过普通 uniform 传入。
// 四个片段着色器（红/绿/蓝/黄）共享此顶点着色器。
// model 矩阵因每个物体不同，仍通过普通 uniform 传入。
#version 330 core
layout (location = 0) in vec3 aPos;

// ⭐ layout(std140) 指定 uniform block 使用 std140 内存布局
// std140 是 OpenGL 定义的标准化布局规则，保证各平台上 uniform 的偏移量一致，
// 使 C++ 端可以精确计算数据在 UBO 中的位置并直接用 glBufferSubData 写入。
// 此 block 包含两个 mat4：projection 和 view，与 C++ 端 UBO 的排列一一对应。
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}