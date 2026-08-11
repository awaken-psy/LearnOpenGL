// 立方体顶点着色器 — 传递法线和世界空间位置用于反射计算
//
// ⭐ 与 6.1 的区别：
//   - 输入包含法线(Normal) 而非纹理坐标
//   - 输出法线和世界空间位置（Position）给片元着色器
//
// ⭐ 法线变换：Normal = mat3(transpose(inverse(model))) * aNormal
//   当模型矩阵有非均匀缩放时，直接用 model 变换法线会导致方向错误。
//   必须使用 model 矩阵逆转置矩阵的 3x3 部分来正确变换法线。

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Position = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
