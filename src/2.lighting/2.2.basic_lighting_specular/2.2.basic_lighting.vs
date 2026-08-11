// 物体的顶点着色器 — 和 2.1 几乎一样,唯一区别:法线改用【法线矩阵】变换
// 焦点:Normal = mat3(transpose(inverse(model))) * aNormal
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    // ⭐ 法线矩阵 = transpose(inverse(model)) 的左上 3x3 部分。
    //   为什么不能直接用 model 变换法线?如果 model 含【非均匀缩放】(x/y/z 缩放比例不同),
    //   直接乘 model 会让法线不再垂直于表面(方向算错)。法线矩阵是数学上正确的法线变换。
    //   本 demo 物体没缩放时它等于单位矩阵,但写成这样更通用、更安全。
    //   mat3(...) 取 4x4 矩阵的左上 3x3(法线是方向,只要旋转/缩放,不要平移分量)。
    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
