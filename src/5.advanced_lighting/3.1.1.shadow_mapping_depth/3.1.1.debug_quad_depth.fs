// 阴影映射(1/3)— 调试着色器:把深度图当灰度图显示出来
//
// 目的:确认第一趟渲染的深度图"长得对不对"。全黑=全在近裁面、全白=全在远裁面,
// 都说明 near/far 没罩住场景。
//
// 深度值的特点:经过【透视投影】后,缓冲里的深度是【非线性】的(近处精度高、远处挤在1.0),
// 直接当亮度会几乎全白,所以透视投影要用 LinearizeDepth 还原成线性。
// 但本demo 用【正交投影】,深度本身就是线性的,直接 vec3(depthValue) 当灰度即可。
// ⭐ 一句话:透视投影才需要 LinearizeDepth,正交投影直接显示。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float near_plane;
uniform float far_plane;

// required when using a perspective projection matrix
// 把 [0,1] 的非线性深度还原成【线性】的视空间深度(透视投影才需要)
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC   // 先把 [0,1] 映射回 [-1,1](NDC)
    // 透视投影的逆变换:linearDepth = (2×n×f) / (f + n − z×(f−n))
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    float depthValue = texture(depthMap, TexCoords).r;
    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic   // ⭐ 正交投影:深度已线性,直接当灰度
}