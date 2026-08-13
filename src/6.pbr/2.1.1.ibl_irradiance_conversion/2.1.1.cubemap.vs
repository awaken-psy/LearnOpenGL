// 立方体顶点着色器 —— 给【环境贴图重投影】和【卷积预计算】用。
//
// 和 background.vs 不同:这里用完整的 projection*view(不抹平移、不做深度技巧)。
// 因为这个着色器是绑在 captureFBO 里渲染到 cubemap 某一面的——是真正"摆相机"画立方体。
//
// WorldPos = 顶点位置,作为方向向量传给 fs,fs 拿它去查 HDR 全景图对应方向的颜色。
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    WorldPos = aPos;
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}