// 高斯模糊片段着色器 — ⭐ 双 pass(水平/垂直)ping-pong 模糊,给 Bloom 用
//
// 一次完整的 2D 模糊分两步:先沿 X 方向、再沿 Y 方向(uniform horizontal 控制)。
// 拆成两个 1D pass 比直接做 2D 卷积快得多 —— 9×9 的 2D 核要 81 次采样,
// 拆成两个 1D 的 9 采样(中心 + 左右各 4)只要 18 次,效果相同。
//
// 【weight[5]】= 5-tap 高斯归一化权重。中心采样用 weight[0]=0.227,±i 偏移采样用 weight[i]。
//   这 5 个值是高斯函数 e^(-x²/2σ²) 在 x=0,1,2,3,4 处离散化后归一化的结果,
//   两侧(±1,±2,±3,±4)共用同一个 weight[i],所以总共 1+2×4=9 个采样点(等效 9×9 核)。
//   权重和 = weight[0] + 2×(weight[1]+...+weight[4]) = 1.0,保证模糊不改变整体亮度。
//
// ⚠ 单次调用只模糊一个方向。完整的"水平+垂直"由 cpp 外层循环交替完成,
//    共 10 次(5 水平 + 5 垂直),每轮叠加一次,光晕越来越柔和。
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

uniform bool horizontal;
// ⭐ 5 个高斯权重,数组初始化语法 float[](值, 值, ...)。
//   weight[0]=中心点权重,weight[1..4]=两侧偏移点的权重(正负偏移共用)。
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
     vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
     // textureSize(image,0) 返回纹理 (宽,高);1.0/它 = 一个 texel 的 UV 步长。
     vec3 result = texture(image, TexCoords).rgb * weight[0];  // 中心采样 × weight[0]
     if(horizontal)
     {
         // 水平方向:左右各偏移 1~4 个 texel,采样 × weight[i] 累加(共 8 个 + 中心 = 9)
         for(int i = 1; i < 5; ++i)
         {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
         }
     }
     else
     {
         // 垂直方向:上下各偏移 1~4 个 texel
         for(int i = 1; i < 5; ++i)
         {
             result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
             result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
         }
     }
     FragColor = vec4(result, 1.0);
}