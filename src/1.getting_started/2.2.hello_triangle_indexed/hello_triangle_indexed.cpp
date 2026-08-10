/**
 * 1.4 新增内容：
 *   EBO（索引缓冲）— 用编号引用顶点，避免重复存储同一个顶点
 *   glDrawElements — 用索引数组来画的绘制指令
 * 按 ESC 键可以关闭窗口。
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


// ---- 着色器源码（同 1.3，无变化）----

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
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


    // ---- 编译着色器（同 1.3，无变化）----

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


    // ========================================================================
    // EBO（Element Buffer Object，索引缓冲）
    //
    // 问题：用 glDrawArrays 画矩形需要 6 个顶点，其中 2 个是重复的。
    //
    //   三角形1: 顶点0→1→3    三角形2: 顶点1→2→3
    //
    //   0───────1              如果用 glDrawArrays：
    //   │╲      │              vertices[] = {v0, v1, v3,    ← 三角形1
    //   │   ╲   │                           v1, v2, v3};    ← 三角形2
    //   │      ╲│              顶点 v1 和 v3 各存了 2 次 → 浪费显存
    //   3───────2
    //
    // 解决：只存 4 个不重复的顶点，再用一套"索引"来引用它们。
    // ========================================================================

    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // 顶点0：右上角
         0.5f, -0.5f, 0.0f,  // 顶点1：右下角
        -0.5f, -0.5f, 0.0f,  // 顶点2：左下角
        -0.5f,  0.5f, 0.0f   // 顶点3：左上角
    };

    // 索引数组：存的是"用 vertices 里的第几个顶点"，不是坐标。
    // 每 3 个索引组成一个三角形。
    unsigned int indices[] = {
        0, 1, 3,   // 三角形1：右上→右下→左上
        1, 2, 3    // 三角形2：右下→左下→左上（顶点1和3被复用了）
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // glGenBuffers 同样用来创建 EBO — 它不区分缓冲类型，只管"分配一个 ID"。
    glGenBuffers(1, &EBO);

    // ---- 配置 VAO ----
    glBindVertexArray(VAO);

    // ---- VBO：顶点数据 ----
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // ---- EBO：索引数据 ----
    // GL_ELEMENT_ARRAY_BUFFER = 索引缓冲专用的槽位，和 GL_ARRAY_BUFFER 互不干扰。
    // 不需要 glVertexAttribPointer — 索引全是 unsigned int，格式固定，不用描述。
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 顶点属性描述（同 2.1）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 解绑 VBO 是安全的 — glVertexAttribPointer 已经把 VBO 注册到了 VAO 里。
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // ⚠️ 但绝对不能解绑 EBO！EBO 的绑定状态保存在当前 VAO 内部，
    // 如果解绑（glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)），
    // VAO 就会丢失索引缓冲的引用，glDrawElements 找不到索引数据。
    // VBO 可以解绑是因为它的引用被 glVertexAttribPointer 显式记录到了 VAO；
    // EBO 没有类似的注册函数，只能靠"当前绑定了哪个 EBO"来隐式记录。
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  ← 不要解开！

    glBindVertexArray(0);

    // 线框模式
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // ---- 渲染循环 ----
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)：
        // 和 glDrawArrays 的区别：不按 VBO 顺序画，而是按 EBO 里的索引跳着取顶点。
        //   mode    = GL_TRIANGLES
        //   count   = 6 → 取 6 个索引，每 3 个组成 1 个三角形，一共画 2 个三角形
        //   type    = GL_UNSIGNED_INT → 索引数组的元素类型是 unsigned int
        //   indices = 0 → 从 EBO 的第 0 字节开始读索引
        // 如果 VAO 没有绑定 EBO，这里传一个 indices 数组的 CPU 地址也可以（但 EBO 方式更快）。
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
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
