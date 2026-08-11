// 物体的顶点着色器 — 新增:输出【片段世界位置 FragPos】和【法线 Normal】给 fs
// 漫反射需要"片段在哪 + 表面朝哪"两个信息,都在这里算好传给 fs。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;   // 顶点在世界空间的位置(光照在世界空间计算)
out vec3 Normal;    // 顶点的法线(本 demo 物体没缩放,直接传原始法线即可)

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 把顶点位置用 model 矩阵变换到【世界空间】——光照计算统一在世界空间做,
    // 所以 FragPos = model * aPos(而不是直接用 aPos 这个物体局部坐标)。
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = aNormal;   // 本 demo 物体没做非均匀缩放,法线不需要修正(2.2 会改)

    // 注意:gl_Position 用 FragPos(世界空间)再乘 view/projection,等价于 P*V*M*aPos。
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
