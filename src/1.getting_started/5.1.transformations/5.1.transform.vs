// 顶点着色器
// 新增：uniform mat4 transform — 4×4 变换矩阵，把顶点位置变换后再输出
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

// mat4 = 4×4 矩阵（GLSL 内置类型）。由 C++ 端用 GLM 算好后通过 uniform 传入。
// 之所以用 4×4 而不是 3×3，是为了能用一个矩阵同时表达平移/旋转/缩放（齐次坐标）。
uniform mat4 transform;

void main()
{
    // 矩阵 × 向量 = 变换后的新位置。
    // 注意顺序：transform * vec4(aPos,1.0)，矩阵在左，向量在右（列向量惯例）。
    // vec4(aPos, 1.0) 把 vec3 升维，w=1.0 表示这是一个"位置点"而非"方向"。
    gl_Position = transform * vec4(aPos, 1.0);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
