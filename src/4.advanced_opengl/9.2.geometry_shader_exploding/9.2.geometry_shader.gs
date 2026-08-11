// 几何着色器：爆炸效果
// 输入：三角形图元（triangles），每个三角形 3 个顶点
// 输出：三角形带（triangle_strip），最多 3 个顶点（输出顶点数与输入相同）
//
// 核心逻辑：
// 1. 计算三角形的面法线（GetNormal）
// 2. 将每个顶点沿法线方向位移（explode），位移量由时间 uniform 驱动
// 3. 输出位移后的 3 个顶点，形成"爆炸"效果
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
} gs_in[];

out vec2 TexCoords; 

// ⭐ time uniform 驱动爆炸动画的位移量
// C++ 端每帧通过 shader.setFloat("time", ...) 更新
uniform float time;

// 沿法线方向位移顶点位置
// magnitude 控制最大位移量，sin(time) 使位移在 0~1 之间周期变化
vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0;
    // (sin(time) + 1.0) / 2.0 将 sin 的 [-1,1] 映射到 [0,1]
    // 这样模型会周期性地从正常状态膨胀到爆炸状态，再缩回正常
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
}

// ⭐ 计算三角形的面法线
// 通过两条边向量的叉积得到垂直于三角形面的法线方向
// 注意：这里使用的是 clip space 位置（gl_in[].gl_Position）来计算法线，
// 而非模型空间的法线属性，因为爆炸效果需要的是面法线方向
vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

void main() {    
    vec3 normal = GetNormal();

    // 对三角形的每个顶点施加爆炸位移，然后输出
    gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoords = gs_in[0].texCoords;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoords = gs_in[1].texCoords;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoords = gs_in[2].texCoords;
    EmitVertex();
    EndPrimitive();
}