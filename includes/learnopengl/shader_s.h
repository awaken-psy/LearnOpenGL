/**
 * Shader 类（simple 版本）— 封装着色器的编译、链接、激活和 uniform 设置
 *
 * 这是所有后续 demo 的基础工具类。和之前手写 glCreateShader / glCompileShader
 * 等十几行代码相比，现在只需要：
 *
 *   Shader s("顶点着色器.vs", "片段着色器.fs");
 *   s.use();
 *   s.setFloat("time", 1.5f);
 *
 * 类名 Shader 虽然叫"着色器"，实际上管理的是"着色器程序"(program)，
 * 即顶点着色器 + 片段着色器链接后的完整管线。
 *
 * shader_m.h 是本文件的增强版，加了更多 uniform setter（vec2/vec3/mat4 等）。
 * 目前先用这个简单版，理解原理后自然过渡到 shader_m.h。
 */

#ifndef SHADER_H
#define SHADER_H
// ↑ 头文件保护宏，防止同一个头文件被多次 #include 导致重复定义。

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    // 着色器程序的 OpenGL ID，public 是为了方便外部直接查询（比如调试时打印）。
    // 正常情况下通过 use() / setXxx() 间接操作，不需要直接碰 ID。
    unsigned int ID;

    // 构造函数：传入两个 shader 文件路径，内部完成从磁盘读取到链接的全部流程。
    // 参数 vertexPath   — 顶点着色器文件路径（如 "3.3.shader.vs"）
    // 参数 fragmentPath — 片段着色器文件路径（如 "3.3.shader.fs"）
    // 文件路径的相对基准是"当前工作目录"（CMake 已经把 shader 复制到了 exe 旁边）。
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // ---- 第一步：从磁盘读取 shader 文件 ----
        // GLSL 源码不再嵌在 C++ 字符串里，而是独立存放在 .vs / .fs 文件中。
        // 用 C++ 标准库 ifstream 读取，和读文本文件完全一样。
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        // exceptions() 设置异常掩码：failbit = 逻辑错误（如读取失败），
        // badbit = 流本身损坏。一旦触发，ifstream 会抛出 std::ifstream::failure 异常，
        // 被下面的 catch 捕获，而不是默默返回空字符串。
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);

            // stringstream：内存中的"流"，可以像文件一样往里写。
            // rdbuf()：获取文件流的底层缓冲区指针。
            // << 操作符：把文件内容整个灌入 stringstream。
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            // str()：把 stringstream 的内容转成 std::string。
            vertexCode   = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            // 即使读取失败也继续执行——vertexCode 为空字符串，后面编译阶段会报错并打印日志，
            // 不至于静默失败。
        }

        // c_str()：string → C 风格字符串（const char*），OpenGL API 需要。
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // ---- 第二步：编译顶点着色器 ----
        unsigned int vertex;
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        // ---- 第三步：编译片段着色器 ----
        unsigned int fragment;
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        // ---- 第四步：创建程序 → 附着 → 链接 ----
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // ---- 第五步：清理中间着色器对象 ----
        // 已经链接进 program 了，单独的 shader 对象不再需要。
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // use() — 激活此着色器程序（等价于 glUseProgram(ID)）。
    // 后续所有 glDrawXxx 调用都会用这个程序的 shader。
    void use()
    {
        glUseProgram(ID);
    }

    // ---- uniform setter 便捷方法 ----
    // 把之前要写两行的 glGetUniformLocation + glUniformXxx 合并成一行。
    // 参数 name — uniform 变量在 shader 里的名字，必须和 GLSL 源码里完全一致。
    // const 后缀 = 这个成员函数不会修改类的成员变量（只读操作）。

    void setBool(const std::string &name, bool value) const
    {
        // glUniform1i：用 1i 是因为 GLSL 里没有真正的 bool 类型，
        // 底层存储还是 int（0=GL_FALSE, 1=GL_TRUE）。
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

private:
    // checkCompileErrors() — 编译/链接错误检查+打印，避免重复写相同的检查代码。
    // 参数 shader — 着色器 ID 或程序 ID（取决于 type）
    // 参数 type   — "VERTEX" / "FRAGMENT" / "PROGRAM"，只影响报错信息的标签
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            // 着色器编译检查
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type
                          << "\n" << infoLog
                          << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            // 程序链接检查
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type
                          << "\n" << infoLog
                          << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

#endif
