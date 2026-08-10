/**
 * 1.5 新增内容：
 *   uniform 变量              — CPU 端能给 shader 传值的全局变量
 *   glGetUniformLocation      — 查询 uniform 变量在 shader 中的"地址"
 *   glUniform4f               — 向 uniform 写入一个 vec4 值
 *   glfwGetTime()             — 获取程序已运行的秒数（时钟源）
 *
 * 效果：三角形的颜色随时间变化（绿色分量按正弦波脉动）。
 * 按 ESC 键可以关闭窗口。
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>    // sin()

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 顶点着色器（无变化）
const char *vertexShaderSource ="#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "}\0";

// 片段着色器 — 新增 uniform 变量
// uniform 是 CPU → GPU 的单向通道：C++ 端 set 一个值，shader 端就能读到。
// 所有着色器阶段都能访问同一个 uniform，但它在一个 draw call 内是常量。
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor;\n"      // ← CPU 可以每帧修改这个值，shader 只读
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"    // 直接用 uniform 的值作为颜色
    "}\n\0";

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

    // ---- 编译着色器（同 1.3）----
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ---- 顶点数据（单个三角形）----
    float vertices[] = {
         0.5f, -0.5f, 0.0f,  // 右下
        -0.5f, -0.5f, 0.0f,  // 左下
         0.0f,  0.5f, 0.0f   // 顶
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 不在循环里重复解绑/绑定 VAO，直接 keep it bound。
    glBindVertexArray(VAO);


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 必须先激活 shader program，才能向它里面的 uniform 写值
        glUseProgram(shaderProgram);

        // ---- 每帧计算并更新 uniform ----
        // glfwGetTime()：返回 glfwInit 以来的秒数（double），是 OpenGL 里的"时钟"。
        // sin() 的取值范围是 [-1, 1]，/2+0.5 映射到 [0, 1]，正好对应颜色分量范围。
        double  timeValue = glfwGetTime();
        float greenValue = static_cast<float>(sin(timeValue) / 2.0 + 0.5);

        // glGetUniformLocation(GLuint program, const GLchar *name)：
        // 在 shader program 里查找名为 "ourColor" 的 uniform 变量，返回它的位置 ID。
        // 找不到则返回 -1（shader 优化掉了未使用的 uniform 时会 -1）。
        int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");

        // glUniform4f(GLint location, float v0, float v1, float v2, float v3)：
        // 向 location 处的 vec4 uniform 写入 4 个 float 值。
        // OpenGL 有一整套 glUniformXxx 函数：
        //   glUniform1f — 1 个 float      glUniform2f — 2 个 float
        //   glUniform4f — 4 个 float      glUniform1i — 1 个 int
        //   glUniformMatrix4fv — 4x4 矩阵
        // 后缀 f = float, i = int, v = vector(数组版本)
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

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
