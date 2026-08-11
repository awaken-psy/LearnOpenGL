// 几何着色器：法线可视化
// 输入：三角形图元（triangles），每个三角形 3 个顶点
// 输出：线段带（line_strip），每个顶点生成 2 个顶点组成一条法线线段
//
// 为三角形的每个顶点生成一条从顶点位置出发、沿法线方向的线段，
// 用于在屏幕上直观显示法线方向。MAGNITUDE 控制线段长度。
#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

// ⭐ 法线线段的长度，值越大法线线段越长
const float MAGNITUDE = 0.2;

uniform mat4 projection;

// 为第 index 个顶点生成一条法线线段（2 个顶点）
void GenerateLine(int index)
{
    // 线段起点：顶点位置（投影变换后）
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    // 线段终点：顶点位置 + 法线方向 × MAGNITUDE
    // 法线已经在顶点着色器中被法线矩阵变换到 view space
    gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    // 为三角形的 3 个顶点各生成一条法线线段
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}