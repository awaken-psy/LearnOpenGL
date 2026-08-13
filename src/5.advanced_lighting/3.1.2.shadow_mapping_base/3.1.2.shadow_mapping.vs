// 阴影映射(2/3)— 主场景顶点着色器
//
// 新增关键输出:【FragPosLightSpace】= 把片段世界坐标再用 lightSpaceMatrix 变换一遍,
// 得到"该片段在【光源空间】下的位置"。fs 拿它和深度图比对才能判断是否在阴影里。
//
// 用【接口块 interface block】(out VS_OUT {...} vs_out) 把多个变量打包传给 fs,
// fs 端用同名块 in VS_OUT {...} fs_in; 接收。比逐个声明更整洁。
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;   // ⭐ 片段在光源空间下的坐标(vec4,保留 w 做透视除法)
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    // 法线矩阵 = mat3(transpose(inverse(model)));非均匀缩放时法线方向才不会歪
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    // ⭐ 把世界空间片段坐标变换到光源空间——阴影判断的关键输入。
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}