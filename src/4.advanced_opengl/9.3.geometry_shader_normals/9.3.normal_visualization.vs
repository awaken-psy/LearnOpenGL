// 顶点着色器：法线可视化
// 计算法线矩阵并变换法线，将变换后的法线和位置传递给几何着色器。
// 注意：此着色器的 gl_Position 只乘了 view*model（未乘 projection），
// 因为投影变换会交给几何着色器完成。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main()
{
    // ⭐ 法线矩阵 = transpose(inverse(mat3(view * model)))
    // 之所以要用 inverse+transpose，是因为当模型有非均匀缩放时，
    // 直接用 model 矩阵变换法线会导致法线方向错误。
    // 法线矩阵确保法线在变换后仍垂直于表面。
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vs_out.normal = vec3(vec4(normalMatrix * aNormal, 0.0));
    gl_Position = view * model * vec4(aPos, 1.0); 
}