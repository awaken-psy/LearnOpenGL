// HDR 色调映射片段着色器 — ⭐ 把【浮点颜色】([0,∞)) 压回显示器能显示的 [0,1]
//
// 输入 hdrBuffer 是上一 pass 渲染到浮点 FBO 的场景纹理,RGB 可能远大于 1.0(光源处几百)。
// 显示器只能显示 [0,1],>1 的值会被直接截断成纯白,大片细节糊在一起。
// 【色调映射 tone mapping】用一条曲线把 [0,∞) 平滑压回 [0,1],再 gamma 校正输出。
//
// 两种 tonemap(用 uniform hdr 开关切换):
//   - Reinhard:color/(color+1)。最简单,曲线对称,无论 input 多大都压不到 1。
//   - Exposure:1 - exp(-color·exposure)。exposure 越大画面越亮,可实时调,更灵活。
//
// 注:本 shader 处理的是【全屏四边形】(NDC 坐标,第 4 章学过),不做 3D 变换。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;            // = false 时跳过 tonemap(只 gamma,对比看 HDR 的作用)
uniform float exposure;      // 【曝光】exposure tonemap 的强度,cpp 里 Q/E 实时调

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    if(hdr)
    {
        // Reinhard tonemap(本 demo 注释掉,换用 exposure):
        //   result = color/(color+1)。优点是简单稳定;缺点是没有可调参数,无法控制曝光。
        // reinhard
        // vec3 result = hdrColor / (hdrColor + vec3(1.0));
        // exposure
        // ⭐ Exposure tonemap:result = 1 - exp(-hdrColor·exposure)。
        //   直觉:像相机曝光。exposure 越大,-hdrColor·exposure 越负,exp(它) 越接近 0,
        //         1-exp(它) 越接近 1,亮区扩大、整体变亮。
        //   数学:x→∞ 时 result→1;x=0 时 result=0,把 [0,∞) 平滑映到 [0,1)。
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        // also gamma correct while we're at it
        // ⚠ tonemap 之后必须 gamma 校正:输出前要把线性颜色转回 sRGB(显示器是反 gamma 的)。
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else
    {
        // hdr=false:跳过 tonemap,直接 gamma。>1 的颜色被截断成难看的纯白块(对比效果)。
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
}