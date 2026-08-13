// 深度遍的片段着色器 — ⭐【手写 gl_FragDepth】存【线性距离 / far_plane】
//
// ⚠ 关键差异(对比 2D 阴影):
//   2D 阴影图是【硬件自动写深度】,写的是 z/w(非线性,近处精度高远处精度低);
//   点光源阴影要存的是【片段到光源的 3D 距离】,硬件不知道这玩意,必须我们自己算、自己写。
//   所以本 fs 显式设置 gl_FragDepth = 距离 / far_plane,归一化到 [0,1] 方便存进深度缓冲。
//   后续主 fs 采样时,读出值 × far_plane 就能还原成真实距离。
#version 330 core
in vec4 FragPos;  // 从 GS 传来的世界空间位置

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    // 片段到光源的【真实 3D 距离】(欧氏长度)。
    float lightDistance = length(FragPos.xyz - lightPos);

    // 除以 far_plane,把距离映射到 [0,1]。越远越接近 1,最近为 0。
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;

    // ⭐ 显式写 gl_FragDepth:覆盖硬件默认的非线性深度。
    //   这样 depth cubemap 里存的就是"到光源的归一化距离",而不是裁剪空间的 z/w。
    // write this as modified depth
    gl_FragDepth = lightDistance;
}