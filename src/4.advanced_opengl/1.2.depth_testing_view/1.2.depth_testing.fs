// 1.2.depth_testing.fs —— 深度可视化演示的片段着色器
//
// ⭐ 核心概念：【深度缓冲】中的值是非线性的
// 透视投影后，深度值被映射到 [0,1] 范围，但这个映射是非线性的——
// 近裁剪面附近分配了大量精度，远裁剪面附近精度极低。
// 这种非线性映射在数学上是正确的（保证透视正确性），但无法直接用于可视化。
//
// LinearizeDepth() 函数将非线性深度值还原为线性深度值，
// 再除以 far 得到 [0,1] 范围的归一化值，最终映射为灰度颜色：
// - 近处 → 亮（白色，接近 1.0）
// - 远处 → 暗（黑色，接近 0.0）
#version 330 core
out vec4 FragColor;

float near = 0.1; 
float far = 100.0; 

// ⭐ 将非线性深度值【线性化】
// 原理：深度缓冲中的值是经过透视投影变换的非线性值，
// 先从 [0,1] 映射回 NDC 的 [-1,1] 范围，再逆推出真实的线性深度。
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    // ⭐ 获取当前片段的深度值（gl_FragCoord.z 是非线性的），
    // 线性化后除以 far，归一化到 [0,1] 用于可视化
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far to get depth in range [0,1] for visualization purposes
    FragColor = vec4(vec3(depth), 1.0);
}