/**
 * 1.9 新增内容：
 *   多纹理混合        — 同时加载两张纹理，在 shader 里混在一起
 *   glActiveTexture   — 切换纹理单元（GPU 同时支持多个纹理槽位）
 *   mix()             — GLSL 内置函数，按比例混合两个值
 *   stbi_set_flip_vertically_on_load — Y 轴翻转，解决贴图上下颠倒问题
 *
 * 和 4.1 的区别：4.1 只有一张纹理。这里加载了两张（木板 + 笑脸），
 * 用 mix() 按 80%/20% 混合后输出。
 *
 * 效果：木板上叠着一个半透明的笑脸。
 * 按 ESC 键可以关闭窗口。
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <learnopengl/filesystem.h>
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

    Shader ourShader("4.2.texture.vs", "4.2.texture.fs");

    // ---- 顶点数据（同 4.1：位置 + 颜色 + 纹理坐标 × 4 个顶点）----
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // ========================================================================
    // 加载纹理 1 — 木板（container.jpg）
    // ========================================================================
    unsigned int texture1, texture2;

    // ---- 纹理 1 ----
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // 设置环绕和过滤（同 4.1）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // stbi_set_flip_vertically_on_load(bool)：
    // OpenGL 纹理坐标的 Y 轴原点在图片底部，而大多数图片格式（jpg/png）的原点在顶部。
    // 不翻转 → 贴图上下颠倒。
    // 设为 true 后 stbi_load 在解码时自动把每一行翻转，贴图方向就对了。
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(FileSystem::getPath("resources/textures/container.jpg").c_str(),&width, &height, &nrChannels, 0);
    if (data)
    {
        // container.jpg 没有 alpha 通道 → 用 GL_RGB
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);


    // ========================================================================
    // 加载纹理 2 — 笑脸（awesomeface.png）
    // 和纹理 1 的区别：PNG 带透明通道 → 用 GL_RGBA 格式
    // ========================================================================
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load(FileSystem::getPath("resources/textures/awesomeface.png").c_str(),&width, &height, &nrChannels, 0);
    if (data)
    {
        // PNG 带 alpha 通道 → internalformat 和 format 都要用 GL_RGBA（4 通道）。
        // 如果这里写了 GL_RGB，透明区域会变黑或花掉。
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);


    // ---- 绑定采样器到纹理单元 ----
    // 着色器里的 sampler2D 变量不知道"去哪个纹理单元取数据"。
    // 需要通过 uniform 告诉它：texture1 用单元 0，texture2 用单元 1。
    //
    // glUniform1i(location, int)：
    // sampler2D 虽然存的是纹理，但 uniform 的类型是 int（纹理单元编号）。
    // 所以用 glUniform1i 而不是 glUniform1f。
    ourShader.use();
    ourShader.setInt("texture1", 0);   // sampler2D texture1 → 纹理单元 0
    ourShader.setInt("texture2", 1);   // sampler2D texture2 → 纹理单元 1


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ---- 绑定纹理到各自的纹理单元 ----
        // glActiveTexture(GLenum texture)：激活指定编号的纹理单元。
        // GL_TEXTURE0 ~ GL_TEXTURE15（至少 16 个槽位，具体上限取决于 GPU）。
        // 之后调用的 glBindTexture 就是把纹理绑到当前激活的单元上。
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);   // 纹理 1 → 单元 0
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);   // 纹理 2 → 单元 1

        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

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
