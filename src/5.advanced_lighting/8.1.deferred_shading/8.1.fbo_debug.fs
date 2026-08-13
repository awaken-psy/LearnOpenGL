// FBO 调试片段着色器 —— 直接采样传入的 FBO 附件(gPosition / gNormal / gAlbedoSpec 等)
// 并输出到屏幕,用来肉眼验证 G-Buffer 是否写对了(比如位置图应该是渐变色)。
// fragment shader
#version 330 core
out vec4 FragColor;
in  vec2 TexCoords;
  
uniform sampler2D fboAttachment;
  
void main()
{
    FragColor = texture(fboAttachment, TexCoords);
} 