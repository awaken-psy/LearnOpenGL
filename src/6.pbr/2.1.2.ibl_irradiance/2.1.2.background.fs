// 天空盒片段着色器 —— 【与 2.1.1.background.fs 相同】。详见 2.1.1 注释。
// 采样 envCubemap + Reinhard tonemap + gamma 校正。
#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

void main()
{		
    vec3 envColor = texture(environmentMap, WorldPos).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    FragColor = vec4(envColor, 1.0);
}
