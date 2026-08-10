// 顶点着色器
// 新增：model / view / projection 三个矩阵，串成完整的坐标变换链
//
// 一个顶点从"模型坐标"到"屏幕坐标"要经过 4 个空间：
//   局部空间 ─model→ 世界空间 ─view→ 观察空间 ─projection→ 裁剪空间 ─(透视除法+视口)→ 屏幕空间
//
// shader 里一行就把前三步做完：
//   gl_Position = projection * view * model * vec4(aPos, 1.0)
//
// 矩阵乘法从右往左作用：先 model（放到世界），再 view（变到相机视角），最后 projection（投影到屏幕）。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

// 三个变换矩阵，由 C++ 用 GLM 算好后传入
uniform mat4 model;       // 模型矩阵：把顶点从"局部坐标"变换到"世界坐标"（放置+旋转物体）
uniform mat4 view;        // 观察矩阵：把世界变换到"相机视角"（移动/旋转相机）
uniform mat4 projection;  // 投影矩阵：把 3D 投影成 2D 屏幕坐标（透视/正交）

void main()
{
    // 注意乘法顺序：projection * view * model（P 在最左，最后应用）
    // 相当于：先把顶点 model 放进世界，再被 view 拉到相机前，最后被 projection 投影。
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
