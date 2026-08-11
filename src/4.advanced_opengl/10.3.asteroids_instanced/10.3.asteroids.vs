// 10.3 实例化小行星顶点着色器
// ⭐ 与 10.2 的关键区别：model 矩阵不再是 uniform，而是【实例化属性】aInstanceMatrix
// 每个实例自动获取自己的 mat4 变换矩阵，无需 CPU 逐个设置

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix; // ⭐ 实例化属性：每个实例的模型矩阵

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
// ⭐ 注意：不再有 uniform mat4 model，因为模型变换已通过实例化属性传入

void main()
{
    TexCoords = aTexCoords;
    // ⭐ 使用实例化矩阵替代 uniform model 矩阵
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0f); 
}
