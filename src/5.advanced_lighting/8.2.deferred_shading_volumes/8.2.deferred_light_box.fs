// Light Box 片段着色器 —— 与 8.1 相同(输出光源颜色,画成可见小光球)
#version 330 core
layout (location = 0) out vec4 FragColor;

uniform vec3 lightColor;

void main()
{           
    FragColor = vec4(lightColor, 1.0);
}