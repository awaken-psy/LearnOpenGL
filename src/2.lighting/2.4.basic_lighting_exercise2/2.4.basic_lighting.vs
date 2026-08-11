// 物体的顶点着色器 — 在观察空间(View Space)做光照
// 与 2.2 的区别:FragPos、Normal、LightPos 全部转到观察空间
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec3 LightPos;

// lightPos 是世界空间坐标,在 vs 里乘 view 转到观察空间后传给 fs
uniform vec3 lightPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // 顶点位置转到观察空间
    FragPos = vec3(view * model * vec4(aPos, 1.0));
    // 法线用 view*model 的法线矩阵变换(观察空间)
    Normal = mat3(transpose(inverse(view * model))) * aNormal;
    // 光源位置也转到观察空间
    LightPos = vec3(view * vec4(lightPos, 1.0));
}
