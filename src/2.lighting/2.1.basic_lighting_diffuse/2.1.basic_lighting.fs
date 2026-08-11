// 物体的片段着色器 — ⭐ 本课核心:漫反射光照
//
// 光照公式(本节版):最终颜色 = (环境光 + 漫反射) × 物体颜色
//
// 漫反射的物理直觉(余弦定律):
//   光线越正对表面 → 单位面积收到光越多 → 越亮
//   光线越斜 → 同一束光摊在更大面积 → 越暗
//   这个"正对程度" = 法线 N 与 光照方向 L 的夹角余弦 = N·L(两个单位向量的点乘)
//
//        N (法线,垂直表面向外)
//        ↑   ╱ L (从片段指向光源)
//        │  ╱
//        │ ╱   ← 夹角越小,N·L 越接近 1 → 越亮
//   ─────┼╱─────── 表面
//
// 三个 uniform:
//   lightPos    — 光源世界位置(用来算 L)
//   lightColor  — 光源颜色
//   objectColor — 物体反射率
#version 330 core
out vec4 FragColor;

in vec3 Normal;   // 来自 vs 的法线(经过光栅化插值)
in vec3 FragPos;  // 来自 vs 的片段世界位置(经过插值)

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // ---- 环境光 ambient ----
    // 给所有面一点均匀底光,模拟"光在场景里弹来弹去的间接光"(这里没真算间接光,
    // 只是经验上拍一个 0.1 倍的光源色)。没有它,背光面会纯黑,看起来不自然。
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // ---- 漫反射 diffuse ----
    vec3 norm = normalize(Normal);
    // lightDir:从片段指向光源的方向(注意是 lightPos - FragPos,不是反过来)。
    vec3 lightDir = normalize(lightPos - FragPos);
    // ⭐ 核心:点乘 norm·lightDir = cos(夹角)。
    //   夹角 0°(光正对着面) → dot=1 → 最亮
    //   夹角 90°(光平行于面) → dot=0 → 不亮
    //   夹角 >90°(光在背面)  → dot<0 → max(·,0) 截断为 0(背光面不该是负亮度)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 最终颜色 = (环境 + 漫反射) × 物体反射率
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}
