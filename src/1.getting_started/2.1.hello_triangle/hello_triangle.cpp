/**
 * 1.3 新增内容：
 *   着色器      — 顶点着色器 + 片段着色器，GPU 上运行的小程序
 *   VBO        — 显存中存放顶点数据的缓冲区
 *   VAO        — 描述顶点数据如何解析的"说明书"
 *   glDrawArrays — 真正发出绘制指令
 * 按 ESC 键可以关闭窗口。
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 顶点着色器源码 — GLSL 语言（类 C），以字符串嵌入 C++ 中。
// GPU 每处理一个顶点，就运行一次这个 shader 的 main()。
// 这里的逻辑极简单：直接把接收到的顶点坐标原封不动传给 gl_Position。
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"   // layout(location=0) → VAO 中属性编号为 0 的数据绑定到 aPos
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"  // gl_Position 是 OpenGL 内置变量，决定顶点最终在屏幕上的位置
    "}\0";

// 片段着色器源码 — GPU 每画一个像素，就运行一次这个 shader 的 main()。
// 这里的逻辑：不管三七二十一，每个像素都输出橙色。
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"                                      // out 声明输出变量，FragColor = 这个像素最终的颜色
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"           // R G B A — 橙色，Alpha=1.0 表示完全不透明
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

    // ========================================================================
    // 着色器编译 & 链接流水线
    // 流程：创建着色器 → 绑定源码 → 编译 → 创建程序 → 附着着色器 → 链接 → 删着色器
    // ========================================================================

    // ---- 顶点着色器 ----
    // glCreateShader(GLenum type)：在 GPU 驱动里创建一个空的着色器对象。
    // 参数 GL_VERTEX_SHADER 表示这是顶点着色器（还有 GL_FRAGMENT_SHADER、GL_GEOMETRY_SHADER 等）。
    // 返回值是 OpenGL 内部的 ID（unsigned int），不是内存指针，之后所有操作都通过这个 ID 引用它。
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length)：
    // 把 GLSL 源码字符串绑定到着色器对象上。
    //   shader  — 着色器 ID
    //   count   — 源码字符串有几段（通常 1）
    //   string  — 指向字符串指针的指针（&vertexShaderSource 即 char**）
    //   length  — 每段字符串的长度，NULL 表示以 \0 结尾
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    // glCompileShader(GLuint shader)：编译着色器。
    // 着色器不是 C++ 编译器编译的，而是由 GPU 驱动在运行时编译的。
    // 所以即使用 #version 330 core 这种老语法，VS 也不会报错 — 它只是一段字符串。
    glCompileShader(vertexShader);

    // 检查编译结果。
    int success;
    char infoLog[512];

    // glGetShaderiv(GLuint shader, GLenum pname, GLint *params)：
    // 查询着色器对象的某个整数状态。
    //   shader — 着色器 ID
    //   pname  — 要查什么（GL_COMPILE_STATUS = 编译成功/失败，
    //            GL_SHADER_TYPE = 着色器类型，GL_DELETE_STATUS = 是否已删除……）
    //   params — 结果写入这里，GL_TRUE(1) 或 GL_FALSE(0)
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog)：
        // 取出编译器的报错信息（GLSL 语法错误等）。
        //   shader    — 着色器 ID
        //   maxLength — infoLog 缓冲区的最大容量（字节），超出的部分会被截断
        //   length    — 实际写入的字节数，填 NULL 代表不关心
        //   infoLog   — 存放报错文本的 char 数组，调用前不需要初始化
        // 不打印这个日志，shader 写错时你只能看到"三角形没出来"，完全不知道问题在哪。
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // ---- 片段着色器 ----
    // 流程和顶点着色器完全一样，只是类型换成 GL_FRAGMENT_SHADER。
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // ---- 链接着色器程序 ----
    // 前面只是编译了两个独立的着色器，它们还互不认识。
    // 需要把"顶点着色器"和"片段着色器"拼成一个程序，GPU 才知道渲染时该一起用它们。
    //
    // glCreateProgram()：创建一个空的着色器程序对象，返回 ID。
    unsigned int shaderProgram = glCreateProgram();

    // glAttachShader(GLuint program, GLuint shader)：把着色器附着到程序上。
    // 可以理解成程序是一个"架子"，把编译好的着色器一个个装上去。
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // glLinkProgram(GLuint program)：链接。
    // 检查顶点着色器的输出和片段着色器的输入是否匹配、所有 uniform 名字是否能对上……
    // 类似 C++ 链接阶段把 .o 文件拼成 .exe。
    glLinkProgram(shaderProgram);

    // 检查链接结果。
    // glGetProgramiv / glGetProgramInfoLog 和着色器版本的函数签名完全一样，
    // 只是第一个参数从 shader ID 换成了 program ID。
    // glGetProgramiv(GLuint program, GLenum pname, GLint *params)：
    //   GL_LINK_STATUS → 链接成功(1) / 失败(0)
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        // glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog)
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // glDeleteShader(GLuint shader)：删除着色器对象，释放 GPU 资源。
    // 参数就是着色器 ID，一次删一个。删完后 GL 内部会将此 ID 标记为未使用，后续可被 glCreateShader 复用。
    // 链接完成后着色器已经"嵌入"了程序，删掉不影响程序运行（程序里已经有编译好的二进制了）。
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // ========================================================================
    // 顶点数据 + VAO + VBO
    // 流水线：定义顶点数组 → 创建 VBO → 上传数据到显存 → 创建 VAO → 描述数据格式 → 绑定
    // ========================================================================

    // NDC（Normalized Device Coordinates，标准化设备坐标）：
    // 范围 x∈[-1, 1], y∈[-1, 1], z∈[-1, 1]。原点在屏幕中心。
    // 只有落在这个范围内的点才会被渲染，范围外的被裁剪掉。
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // 左下角 (x=-0.5, y=-0.5, z=0.0)
         0.5f, -0.5f, 0.0f, // 右下角
         0.0f,  0.5f, 0.0f  // 顶部
    };

    // VBO = Vertex Buffer Object，显存中的一块缓冲区。
    // 类比：VBO 是一个快递包裹，里面装着顶点数据。
    //
    // VAO = Vertex Array Object，"如何解读 VBO 里的数据"的说明书。
    // 类比：同一箱积木（VBO），不同的说明书（VAO）能拼出不同的东西。
    // VAO 记录了：每个顶点有几个属性、每个属性几个分量、数据类型是什么、从哪里开始读……
    unsigned int VBO, VAO;

    // glGenVertexArrays(GLsizei n, GLuint *arrays)：生成 n 个 VAO，ID 写入 arrays。
    // 类似 malloc — 向 OpenGL 申请 VAO，OpenGL 返回一个编号。
    glGenVertexArrays(1, &VAO);

    // glGenBuffers(GLsizei n, GLuint *buffers)：生成 n 个缓冲对象（这里是 VBO），ID 写入 buffers。
    glGenBuffers(1, &VBO);

    // ---- 配置 VAO ----
    // 先绑定 VAO。之后的 glBindBuffer、glVertexAttribPointer、glEnableVertexAttribArray
    // 都会记录到这个 VAO 里。可以理解成"打开这本说明书，开始往里写内容"。
    glBindVertexArray(VAO);

    // ---- 上传顶点数据到 VBO ----
    // glBindBuffer(GLenum target, GLuint buffer)：把 VBO 绑定到 GL_ARRAY_BUFFER 槽位。
    // GL_ARRAY_BUFFER 是专门存放"顶点属性"的缓冲类型，绑定后所有对 GL_ARRAY_BUFFER 的操作
    // 都作用在这个 VBO 上。
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)：
    // 把数据从 CPU 内存传到 GPU 显存。
    //   target — 目标缓冲类型（GL_ARRAY_BUFFER = 顶点属性缓冲）
    //   size   — 数据大小（字节）
    //   data   — 指向数据的指针
    //   usage  — 使用提示，帮助驱动决定把数据放在哪块显存（快还是省？）
    //            GL_STATIC_DRAW  = 设一次、画很多次（数据几乎不变）
    //            GL_DYNAMIC_DRAW = 会频繁修改（游戏中的粒子系统）
    //            GL_STREAM_DRAW  = 每次画都不一样
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // ---- 描述顶点数据格式 ----
    // glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
    //                       GLsizei stride, const void *pointer)：
    // 告诉 OpenGL 如何从 VBO 的字节流里"切出"每个顶点的属性数据。
    //
    //   index      = 0  → 对应着色器中 layout(location=0) 的这个属性
    //   size       = 3  → 每个顶点有 3 个分量（x, y, z）
    //   type       = GL_FLOAT → 每个分量是 float（4 字节）
    //   normalized = GL_FALSE → 不要归一化（颜色值才需要归一化到 0~1，坐标不需要）
    //   stride     = 3 * sizeof(float) = 12 → 从顶点 A 的 x 到顶点 B 的 x 跳过 12 字节
    //   pointer    = (void*)0 → 从 VBO 的第 0 字节开始读
    //
    // 内存布局（stride=12）：
    //   |-- 顶点0 --|-- 顶点1 --|-- 顶点2 --|
    //   | x | y | z | x | y | z | x | y | z |  每个 x/y/z 占 4 字节
    //   0   4   8   12  16  20  24  28  32
    //   ↑           ↑           ↑
    //   顶点0起点    顶点1起点     顶点2起点
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // glEnableVertexAttribArray(GLuint index)：启用 location=0 这个顶点属性。
    // 默认所有顶点属性都是禁用的，必须显式开启，数据才会被送入着色器。
    glEnableVertexAttribArray(0);

    // ---- 解绑 ----
    // VAO 配置完了，解绑 VBO（不解绑 VAO，因为渲染时还要用 VAO 来切换顶点配置）。
    // 如果场景里有多个物体，每个绑自己的 VAO 就行了，不用重复设置 VBO。
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // 接着解绑 VAO，防止后面的代码意外修改它。
    glBindVertexArray(0);

    // 线框模式：取消注释后每个三角形的边会用线画出来，方便理解三角形面片的几何结构。
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // ========================================================================
    // 渲染循环
    // ========================================================================
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // glUseProgram(GLuint program)：激活着色器程序。
        // 后续所有 glDraw* 调用都会用这个程序的 shader。
        // 一个场景可能有多个 shaderProgram（金属的、木头的、水的……），
        // 每画一种材质前切换一次。
        glUseProgram(shaderProgram);

        // glBindVertexArray：绑定 VAO，OpenGL 就知道去哪找顶点数据、每个属性怎么解析。
        // 因为本程序只有一个 VAO，这行其实可以放在循环外面。留着是为以后多物体做准备。
        glBindVertexArray(VAO);

        // glDrawArrays(GLenum mode, GLint first, GLsizei count)：发出绘制指令。
        //   mode  — GL_TRIANGLES：每 3 个顶点组成一个三角形
        //           GL_POINTS：每个顶点画一个点
        //           GL_LINE_STRIP：顶点首尾相连的折线（画轨迹）
        //           GL_TRIANGLE_STRIP：每新增一个顶点和前两个顶点组成三角形（画带状面）
        //   first — 从第几个顶点开始（0 = 第一个）
        //   count — 一共几个顶点（3 = 正好一个三角形）
        glDrawArrays(GL_TRIANGLES, 0, 9);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- 释放 GPU 资源 ----
    // glDeleteVertexArrays / glDeleteBuffers 的参数和 Gen 系列对称：
    //   第一个参数 n = 要删几个
    //   第二个参数 arrays/buffers = 存放 ID 的数组地址（即使只有 1 个也传 &变量名）
    // 内部会先检查 ID 是否有效，只清理有效对象，对已删除或不存在的 ID 静默忽略。
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glDeleteProgram(GLuint program)：删除着色器程序。
    // 只接受一个 program ID（没有 n 参数），一次删一个程序。
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
