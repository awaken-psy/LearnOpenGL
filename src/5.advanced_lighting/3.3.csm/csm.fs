// 占位 demo 的片段着色器 —— 只做【线性化深度并显示为灰度图】,没有任何 CSM 相关逻辑
// (本 demo 本身是空壳,见 csm.cpp 顶部说明)
//
// LinearizeDepth:把透视投影产生的非线性深度(gl_FragCoord.z,近处精度高远处精度低)
// 转换成线性的 [0,1] 距离,这样可视化时近远都看得清楚。这函数在第4章深度测试已学过。
#version 330 core
out vec4 color;

float LinearizeDepth(float depth) // Note that this ranges from [0,1] instead of up to 'far plane distance' since we divide by 'far'
{
    float near = 0.1;
    float far = 100.0;
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near) / (far + near - z * (far - near));
}

void main()
{
    // 把当前片元的深度线性化,作为灰度值输出 → 屏幕上越近越黑、越远越白。
    float depth = LinearizeDepth(gl_FragCoord.z);
    color = vec4(vec3(depth), 1.0f);
}