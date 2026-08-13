// FBO 调试顶点着色器 —— 把任意 FBO 附件贴到全屏 quad 上方便肉眼检查 G-Buffer 内容。
// 输入是 2D 位置 + 纹理坐标(不走 3D MVP),直接输出 NDC。
// vertex shader
#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(position, 0.0f, 1.0f);
    TexCoords = texCoords;
}
 