// 天空盒顶点着色器 —— 第 4 章学过的 skybox 技巧,这里只是用上 IBL 转好的 cubemap。
//
// 两个关键点(第 4 章已学,简单提一下):
//   1. mat4(mat3(view)):取 view 矩阵的【旋转部分】,丢掉平移列。
//      这样天空盒就永远以原点为中心——相机移动时它不会"过去",看起来无限远。
//   2. gl_Position = clipPos.xyww:用 w 覆盖 z,让深度变成 w/w = 1.0(最远平面)。
//      配合主程序里的 glDepthFunc(GL_LEQUAL),天空盒就能写到最远的深度缓冲。
//
// WorldPos 直接 = 顶点位置(单位立方体),作为方向向量传给 fs 去采样 cubemap。
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main()
{
    WorldPos = aPos;

	// ⭐ 抹掉 view 的平移部分(把第 4 列变成 0,0,0,1),天空盒就不跟着相机走了。
	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);

	// ⭐ xyww:用 w 覆盖原本的 z。透视除法后 z/w = 1.0,顶点就落在最远平面。
	//   ⚠ 必须配合 glDepthFunc(GL_LEQUAL),否则被默认 GL_LESS 拒绝写入。
	gl_Position = clipPos.xyww;
}