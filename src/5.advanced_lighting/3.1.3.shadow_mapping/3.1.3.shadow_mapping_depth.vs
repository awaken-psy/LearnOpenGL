// 阴影映射(3/3)— 深度图顶点着色器。内容与 3.1.1 完全相同,详见 3.1.1.shadow_mapping_depth.vs。
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}