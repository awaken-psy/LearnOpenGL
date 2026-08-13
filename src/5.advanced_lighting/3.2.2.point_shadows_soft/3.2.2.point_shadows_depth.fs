// ⚠ 本文件与 3.2.1.point_shadows_depth.fs【完全相同】(手写 gl_FragDepth 存线性距离),
//   原理和每行注释见 3.2.1.point_shadows_depth.fs,这里不重复。
#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}