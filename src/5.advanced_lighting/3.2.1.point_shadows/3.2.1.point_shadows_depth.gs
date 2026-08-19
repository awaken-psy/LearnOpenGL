// 几何着色器 — ⭐ 本 demo 的核心!把每个三角形【复制 6 份】分别写入 cubemap 6 个面
//
// 输入:layout(triangles) in —— 每次进来一个三角形(3 个顶点)。
// 输出:layout(triangle_strip, max_vertices=18) out —— 最多输出 18 个顶点
//       = 6 面 × 3 顶点(每面都重新发一个三角形)。
//
// 工作方式:双层循环。
//   外层 face = 0..5:遍历 cubemap 的 6 个面,gl_Layer = face 指定【后续顶点写到哪个面】。
//   内层 i = 0..2:对该三角形的 3 个顶点,逐个用 shadowMatrices[face] 变换并发送(EmitVertex)。
//   每个面发完 3 个顶点后 EndPrimitive() 结束这个三角形。
//
// 这样【一次 draw call】就完成了 6 个面的深度图渲染,不用在 cpp 端循环 6 次。
// (代价是 GS 阶段的额外处理开销,但比 6 次 draw call + 6 次状态切换便宜得多。)
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];  // 6 个面的【光空间矩阵】(cpp 端算好传进来)

out vec4 FragPos; // FragPos from GS (output per emitvertex)
// FragPos:世界空间片段位置,传给 depth.fs 算"到光源的距离"用。

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        // ⭐ gl_Layer:GS 内置变量,指定【当前 EmitVertex 输出的图元写到 cubemap 的哪一面】。
        //   必须在每个面发顶点【之前】设置,且对 cubemap FBO 才有效。
        //   这就是"一次 draw 写 6 面"的关键 —— 通过切换 gl_Layer 把不同三角形分发到不同面。
        gl_Layer = face; 
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            // gl_in[i].gl_Position 是 vs 输出,这里就是 model × aPos = 【世界坐标】。
            // 把它传给 fs(FragPos),让 fs 算到光源的距离。
            FragPos = gl_in[i].gl_Position;
            // 用【当前面】的 shadowMatrices[face] 把世界坐标变换到该面的裁剪空间。
            // 同一个顶点在每个面会被算 6 次(每面一个矩阵),结果各不相同。
            gl_Position = shadowMatrices[face] * FragPos;
            // EmitVertex():把当前 gl_Position 和所有 out 变量(FragPos)作为一个顶点发送出去。
            EmitVertex();
        }
        // 3 个顶点发完 → 结束这个面的三角形条带。然后进入下一个面,gl_Layer 切换。
        EndPrimitive();
    }
}