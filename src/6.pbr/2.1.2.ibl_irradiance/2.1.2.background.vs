// 天空盒顶点着色器 —— 【与 2.1.1.background.vs 相同】。详见 2.1.1 注释。
// (mat4(mat3(view)) 去平移 + gl_Position=clipPos.xyww 深度技巧)
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main()
{
    WorldPos = aPos;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);

	gl_Position = clipPos.xyww;
}