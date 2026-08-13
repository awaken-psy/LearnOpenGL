// ⚠ 本文件与 3.2.1.point_shadows_depth.vs【完全相同】(只做 model 变换),
//   原理和每行注释见 3.2.1.point_shadows_depth.vs,这里不重复。
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(aPos, 1.0);
}