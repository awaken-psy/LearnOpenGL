// 顶点着色器 — 和 6.1 基本相同，唯一差异：纹理 Y 坐标翻转（1.0 - aTexCoord.y）
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    // 1.0 - aTexCoord.y — 把纹理上下翻转。和 stbi_set_flip_vertically_on_load 作用类似，
    // 但这里是在 shader 里翻，因为本例 10 个立方体位置不同，统一在 shader 处理更方便。
    TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}
