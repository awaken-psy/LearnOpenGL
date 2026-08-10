/**
 * 1.7 新增内容：
 *   Shader 类          — 把着色器的创建、编译、链接、使用、uniform 设值全封装起来
 *   从文件读取 shader   — shader 源码不再嵌在 C++ 字符串里，而是独立的 .vs/.fs 文件
 *
 * 从本课开始，所有后续 demo 都用 Shader 类，不再手写 glCreateShader/glCompileShader。
 * 对应的 shader 源码在同目录下的 3.3.shader.vs 和 3.3.shader.fs 文件中。
 * 按 ESC 键可以关闭窗口。
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Shader 类的头文件，位于 includes/learnopengl/shader_s.h
// s = simple，只有最基础的功能（之后还会用到功能更全的 shader_m.h）。
#include <learnopengl/shader_s.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // ---- Shader 类：一行替代之前十几行 ----
    // 构造函数接受两个文件路径（顶点着色器 + 片段着色器），内部自动完成：
    //   ① 打开文件读取源码
    //   ② glCreateShader → glShaderSource → glCompileShader （两个着色器各一次）
    //   ③ glCreateProgram → glAttachShader → glLinkProgram
    //   ④ glDeleteShader（清理中间对象）
    // 之前 3.1、3.2 里那几十行编译链接代码，现在缩成一行。
    //
    // shader_s.h 的 class Shader 提供的方法：
    //   use()             — glUseProgram(ID)
    //   setBool(name, v)  — glUniform1i(...)
    //   setInt(name, v)   — glUniform1i(...)
    //   setFloat(name, v) — glUniform1f(...)
    Shader ourShader("3.3.shader.vs", "3.3.shader.fs");


    // ---- 顶点数据（同 3.2：位置 + 颜色双属性）----
    float vertices[] = {
        // positions          // colors
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   // 右下 — 红
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   // 左下 — 绿
         0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f    // 顶 — 蓝
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 之前的写法：glUseProgram(shaderProgram)
        // Shader 类封装后：
        ourShader.use();

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // 注意：不需要 glDeleteProgram — Shader 类的析构函数会自动清理。
    // （shader_s.h 没有显式析构函数，但程序退出时 GPU 资源由操作系统回收）

    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
