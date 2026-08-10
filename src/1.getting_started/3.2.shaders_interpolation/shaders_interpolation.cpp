/**
 * 1.6 新增内容：
 *   多个顶点属性            — 每个顶点除了位置，还可以带颜色、法线、纹理坐标等
 *   顶点着色器 out / 片段着色器 in — 顶点之间传递数据
 *   插值（Interpolation）   — GPU 自动在两个顶点之间平滑过渡颜色
 *
 * 和 1.5 的区别：1.5 用 uniform 给整个三角形指定单一颜色。
 * 这里每个顶点自带颜色，GPU 在面片内部自动渐变（三个顶点红绿蓝 → 中间是渐变色）。
 * 按 ESC 键可以关闭窗口。
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 顶点着色器 — 现在有两个输入属性
// layout(location=0) → aPos（位置）
// layout(location=1) → aColor（颜色）
// 顶点着色器把颜色通过 out 变量传给片段着色器
const char *vertexShaderSource ="#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"     // location=0：位置属性
    "layout (location = 1) in vec3 aColor;\n"   // location=1：颜色属性
    "out vec3 ourColor;\n"                       // 输出给片段着色器的变量，名字不限
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"                    // 把颜色原封不动往下传
    "}\0";

// 片段着色器 — 接收来自顶点着色器的插值后颜色
// 关键：顶点着色器的 out 变量名 必须和 片段着色器的 in 变量名 一致
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"                        // 接收顶点着色器传下来的颜色
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0f);\n"
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


    // ---- 顶点数据：位置 + 颜色交错存储 ----
    // 每个顶点现在有 6 个 float，前 3 个是位置，后 3 个是颜色。
    //
    // VBO 内存布局（stride = 24 字节）：
    //   |--- 顶点0 ---||--- 顶点1 ---||--- 顶点2 ---|
    //   | x | y | z | R | G | B | x | y | z | R | G | B | ...
    //   0   4   8   12  16  20  24  28  32  36  40  44
    //                ↑                            ↑
    //         颜色从第12字节开始              颜色从第36字节开始
    float vertices[] = {
        // 位置(x,y,z)          颜色(R,G,B)
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   // 右下 — 红色
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   // 左下 — 绿色
         0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f    // 顶部 — 蓝色
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // ---- 属性 0：位置（location=0）----
    // stride = 6 * sizeof(float) = 24 字节 — 每跳过一个顶点要跨过 6 个 float
    // offset = 0 — 从 VBO 第 0 字节开始读
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ---- 属性 1：颜色（location=1）----
    // stride 不变（还是 24 字节），但 offset = 3 * sizeof(float) = 12 字节
    // 即跳过前 3 个 float（位置），从第 4 个 float 开始读颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // shader 只激活一次就够了（只有一个 program）
    glUseProgram(shaderProgram);


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
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
