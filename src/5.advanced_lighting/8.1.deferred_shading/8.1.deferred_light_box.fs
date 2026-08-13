// Light Box 片段着色器 — 输出光源本身的颜色,画成可见的小光球(让用户能看到光在哪)
#version 330 core
layout (location = 0) out vec4 FragColor;

uniform vec3 lightColor;

void main()
{           
    FragColor = vec4(lightColor, 1.0);
}