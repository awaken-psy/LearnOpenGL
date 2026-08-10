/**
 * 1.2 新增内容：
 *   glClearColor — 设置"清屏颜色"，每次清屏时把画面刷成什么颜色
 *   glClear      — 执行清屏操作，把当前帧缓冲清空
 * 按 ESC 键可以关闭窗口。
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;   // 宽
const unsigned int SCR_HEIGHT = 600;  // 高

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

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // glClearColor(R, G, B, A)：设置清屏颜色，RGBA 范围都是 0.0 ~ 1.0。
        // 它只设置"用哪个颜色清屏"，并不真的去清屏。
        // OpenGL 会记住这个颜色，直到你再次调用 glClearColor 改变它。
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        // glClear：执行真正的清屏操作，用 glClearColor 设置的颜色把整个画面填满。
        // GL_COLOR_BUFFER_BIT → 清除颜色缓冲（画面）。
        // 除此之外还有 GL_DEPTH_BUFFER_BIT（深度缓冲）、GL_STENCIL_BUFFER_BIT（模板缓冲）。
        // 把 glClearColor 和 glClear 分开，是为了"设置一次颜色，反复清屏时不用每次指定"。
        // GL_COLOR_BUFFER_BIT 是一个标志位，告诉 glClear "我要清的是颜色缓冲这块区域"。 
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
