// 几何着色器：从单个点生成房屋形状
// 输入：点图元（points），每个输入点包含一个顶点
// 输出：三角形带（triangle_strip），最多 5 个顶点组成房子
//
// 房子结构（5 个顶点）：
//   4 --- 5(屋顶顶点)
//   |  /  |
//   1 --- 2  (底部)
//   墙壁用顶点颜色，屋顶固定为白色
#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

// ⭐ 几何着色器的输入变量必须是数组，因为一个图元可能包含多个顶点
// 此处输入是点图元，数组长度始终为 1
in VS_OUT {
    vec3 color;
} gs_in[];

// 输出给片段着色器的颜色
out vec3 fColor;

// 构建房子：以输入点的位置为中心，生成 5 个顶点
void build_house(vec4 position)
{    
    // 墙壁部分使用输入顶点携带的颜色
    fColor = gs_in[0].color; // gs_in[0] since there's only one input vertex
    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0); // 1:bottom-left   
    EmitVertex();   
    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0); // 2:bottom-right
    EmitVertex();
    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0); // 3:top-left
    EmitVertex();
    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0); // 4:top-right
    EmitVertex();
    // ⭐ 在 EmitVertex 之前修改输出变量，可以给不同顶点设置不同属性
    // 这里将屋顶顶点的颜色改为白色
    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0); // 5:top
    fColor = vec3(1.0, 1.0, 1.0);
    EmitVertex();
    EndPrimitive();
}

void main() {    
    build_house(gl_in[0].gl_Position);
}