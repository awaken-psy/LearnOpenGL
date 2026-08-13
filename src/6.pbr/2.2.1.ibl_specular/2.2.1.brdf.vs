// BRDF LUT 顶点着色器 —— 给 renderQuad() 用。
//
// 简单到几乎没有内容:quad 顶点直接当 NDC 坐标(gl_Position = vec4(aPos, 1.0)),
// 把 UV 传给 fs。fs 用 UV 当作 (NdotV, roughness) 入参算 BRDF。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
	gl_Position = vec4(aPos, 1.0);
}