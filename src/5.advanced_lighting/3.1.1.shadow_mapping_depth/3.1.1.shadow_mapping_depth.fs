// 阴影映射(1/3)— 深度图片段着色器:【空操作】
//
// 这个 fs 故意什么都不做。为什么?
//   开启深度测试后,光栅化阶段会自动比较每个片段的 z,把【最近】的深度写进深度缓冲。
//   我们把这个 FBO 的深度缓冲挂成了 depthMap 纹理,所以深度被自动烤进纹理里了。
//   不需要手动输出 gl_FragDepth(那反而会关掉硬件的 early-z 优化)。
#version 330 core

void main()
{
    // gl_FragDepth = gl_FragCoord.z;
}