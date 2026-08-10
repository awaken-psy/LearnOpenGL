/**
 * 1.1涉及的核心概念：
 *   GLFW    — 负责创建窗口、处理键盘/鼠标输入
 *   GLAD    — 负责加载 OpenGL 函数指针（OpenGL 本身只是一套规范，没有实现）
 *   渲染循环 — 游戏/图形程序的"心跳"，每帧执行一次
 *   双缓冲  — 消除画面撕裂的技术
 * 按 ESC 键可以关闭窗口。
 */

// glad 必须放在最前面！它包含了 OpenGL 的所有函数声明。
// 如果把它放在 glfw3 后面，编译会出错（因为 GLFW 的 glad 宏冲突）。
#include <glad/glad.h>

// GLFW (Graphics Library FrameWork)：一个专门为 OpenGL 设计的 C 库，
// 帮我们处理"创建窗口"和"接收输入"这些与操作系统打交道的事情。
// 每个操作系统创建窗口的方式都不同，GLFW 帮我们抹平了这些差异。
#include <GLFW/glfw3.h>

#include <iostream>

// 窗口大小变化时的回调函数。
// 比如你拖拽窗口边缘，宽高变了，这个函数就会被 GLFW 自动调用。
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// 每帧处理键盘输入。
void processInput(GLFWwindow* window);


const unsigned int SCR_WIDTH  = 800;   // 宽
const unsigned int SCR_HEIGHT = 600;   // 高


int main()
{
    // glfwInit() 会初始化 GLFW 库的内部状态。
    glfwInit();

    // glfwWindowHint(选项名, 值)：设置窗口创建时的选项。
    // OpenGL 主版本号 = 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // OpenGL 次版本号 = 3  →  也就是要求 OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // 使用"核心模式"(Core Profile)。
    // 核心模式 = 只提供现代的、推荐使用的 OpenGL 功能，学习 OpenGL 一定要用核心模式，否则会学到过时写法。
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfwCreateWindow(宽, 高, "标题", 全屏模式的显示器(nullptr=窗口模式), 共享资源的窗口(nullptr=不共享))
    // 返回值：指向 GLFWwindow 的指针，创建失败返回 NULL。
    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH, SCR_HEIGHT,     // 800 x 600
        "LearnOpenGL",             // 窗口标题栏显示的文字
        NULL,                      // 全屏才填，窗口模式填 NULL
        NULL                       // 资源共享，暂时不需要
    );

    // 如果窗口创建失败（比如显卡不支持 OpenGL 3.3），打印错误并退出。
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();  // 清理 GLFW 资源
        return -1;
    }

    // 将新窗口设为"当前上下文"，OpenGL 是一个状态机，所有操作都作用于"当前上下文"(Context)。
    // 理解成：OpenGL 是一支笔，上下文就是"当前在画的那张纸"。
    // 不设置上下文的话，OpenGL 不知道往哪里画。
    glfwMakeContextCurrent(window);


    // OpenGL 本身只是一个规范文档，没有具体实现。
    // 真正实现 OpenGL 的是显卡驱动，每个驱动提供的函数地址不同。
    // GLAD 的作用：在运行时向显卡驱动查询所有 OpenGL 函数的地址。
    // glfwGetProcAddress：GLFW 提供的函数，返回当前平台下某个函数的地址。
    // gladLoadGLLoader：GLAD 提供的函数，批量加载所有我们需要的 OpenGL 函数。
    // 这一步不成功 = OpenGL 完全不能工作 = 必须退出。
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 告诉 GLFW："当窗口大小变化时，请调用 framebuffer_size_callback 这个函数"。
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 渲染循环（Render Loop）—— 程序的核心
    // 一直循环，每一帧刷新一次画面。
    while (!glfwWindowShouldClose(window))
    {
        // ---- 处理输入 ----
        processInput(window);

        // ---- 渲染（本课为空，后续课程会在这里画东西）----

        // ---- 交换缓冲区（Swap Buffers） ----
        // OpenGL 使用双缓冲技术来避免画面撕裂：
        //
        //   [前缓冲] — 屏幕上正在显示的图像（观众看到的）
        //   [后缓冲] — 正在绘制的图像（幕后在画的）
        //
        // 如果只有一块缓冲，你在修改画面的同时屏幕也在读取，就会出现"画到一半就被显示"的撕裂现象。
        // glfwSwapBuffers 的作用：把后缓冲的内容交换到前缓冲去显示，把前缓冲变成后缓冲供下一帧绘制。
        glfwSwapBuffers(window);

        // ---- 处理系统事件 ----
        // 检查有没有键盘按下、鼠标移动、窗口大小变化等事件，
        // 如果有，触发对应的回调函数（比如 framebuffer_size_callback）。
        glfwPollEvents();
    }

    // 循环结束 ← 用户点了 × 或按了 ESC
    // 释放 GLFW 分配的所有资源（窗口、内存等）。
    glfwTerminate();


    return 0;
}


void processInput(GLFWwindow* window)
{
    // glfwGetKey(窗口, 要检查的键)：
    //   如果该键正被按下 → 返回 GLFW_PRESS
    //   如果该键没被按下 → 返回 GLFW_RELEASE
    // 这里检查 ESC 键，按了就关闭窗口。
    // glfwSetWindowShouldClose: 告诉 GLFW "这个窗口该关了"。
    // 下一次 while 条件检查 glfwWindowShouldClose 时就会返回 true，循环退出。
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


// 什么时候触发？
//   - 用户拖拽窗口边缘
//   - 用户点击最大化/还原按钮
//   - 系统 DPI 缩放变化（比如插拔外接显示器）
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // glViewport(左下角x, 左下角y, 宽, 高)：
    // 告诉 OpenGL 渲染区域的大小和位置，通常 (0, 0) 就是窗口左下角，宽高等于窗口大小。
    // 如果不调这个函数，窗口拉伸后画面会变形或者只画在窗口一角。
    glViewport(0, 0, width, height);
}
