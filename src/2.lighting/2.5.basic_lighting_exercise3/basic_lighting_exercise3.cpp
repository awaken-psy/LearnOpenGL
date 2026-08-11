/**
 * 练习 2.5 — Gouraud 着色 vs Phong 着色
 *
 * 之前(2.1~2.2)在【片段着色器】里算光照,叫 Phong 着色——每个像素独立算光照,效果平滑。
 * 本练习反过来:在【顶点着色器】里算光照(Gouraud 着色),再把算好的颜色插值给三角形内部。
 *
 * 后果:立方体正面会出现一道明显的对角线"条纹"。因为高光只在顶点处算出来,
 * 然后线性插值给三角形内的片段;拼成同一个面的两个三角形,插值结果在接缝处对不上。
 * ——这恰好说明"为什么要在片段级算光照(Phong)"。
 *
 * ⚠ 下方是 GLSL 着色器源码,用 #if 0 屏蔽,仅供阅读。
 *
 * 核心区别:光照公式从 fs 搬到了 vs,vs 算出 LightingColor 后直接当 out 插值;
 *          fs 只做 FragColor = LightingColor * objectColor,不再碰光照公式。
 */

#if 0
// Vertex shader:
// ================
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 LightingColor; // resulting color from lighting calculations

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // gouraud shading
    // ------------------------
    vec3 Position = vec3(model * vec4(aPos, 1.0));
    vec3 Normal = mat3(transpose(inverse(model))) * aNormal;

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - Position);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    float specularStrength = 1.0; // this is set higher to better show the effect of Gouraud shading
    vec3 viewDir = normalize(viewPos - Position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    LightingColor = ambient + diffuse + specular;
}


// Fragment shader:
// ================
#version 330 core
out vec4 FragColor;

in vec3 LightingColor;

uniform vec3 objectColor;

void main()
{
   FragColor = vec4(LightingColor * objectColor, 1.0);
}
#endif

/*
条纹现象的原理(教程原文译文):
你会看到立方体正面有一条清晰的两个三角形分界线。这条"条纹"是片段插值造成的。
正面右上角顶点恰好被高光照亮,而同一个三角形(右下三角形)的另外两个顶点没有高光,
于是亮值从右上顶点向另外两个顶点插值衰减;左上三角形同理。由于中间片段的颜色不是
直接来自光照计算,而是插值结果,两个三角形在接缝处的亮度对不上,形成可见条纹。
形状越复杂,这个效应越明显——这就是为什么现代渲染普遍用 Phong(片段级)而非 Gouraud。

原文:
So what do we see?
You can see (for yourself or in the provided image) the clear distinction of the two triangles at the front of the
cube. This 'stripe' is visible because of fragment interpolation. From the example image we can see that the top-right
vertex of the cube's front face is lit with specular highlights. Since the top-right vertex of the bottom-right triangle is
lit and the other 2 vertices of the triangle are not, the bright values interpolates to the other 2 vertices. The same
happens for the upper-left triangle. Since the intermediate fragment colors are not directly from the light source
but are the result of interpolation, the lighting is incorrect at the intermediate fragments and the top-left and
bottom-right triangle collide in their brightness resulting in a visible stripe between both triangles.

This effect will become more apparent when using more complicated shapes.
*/
