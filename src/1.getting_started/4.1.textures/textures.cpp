/**
 * 1.8 新增内容：
 *   纹理（Texture）— 把图片贴到三角形上，代替纯色
 *   stb_image.h       — 轻量图片加载库，读取 jpg/png 等格式
 *   glTexImage2D      — 把像素数据上传到 GPU 作为纹理
 *   纹理坐标          — 新的顶点属性(location=2)，指定"图片的哪个位置贴到顶点上"
 *   sampler2D         — 片段着色器里的纹理采样器
 *
 * 效果：一个贴着木板纹理的矩形，四个角带颜色渐变。
 * 按 ESC 键可以关闭窗口。
 *
 * 新 include: stb_image.h — 单头文件图片库，只需要 #include 然后在某个 .cpp 里 #define STB_IMAGE_IMPLEMENTATION 即可使用。
 *             filesystem.h — FileSystem::getPath() 定位资源文件（纹理、模型等），
 *                            自动处理 exe 路径 vs 仓库路径的差异。
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

    Shader ourShader("4.1.texture.vs", "4.1.texture.fs");

    // ---- 顶点数据：位置(3) + 颜色(3) + 纹理坐标(2) = 每顶点 8 个 float ----
    //
    // 纹理坐标范围 [0.0, 1.0]，(0,0) = 图片左下角，(1,1) = 图片右上角。
    // OpenGL 里图片的 Y 轴从下往上，而大多数图片格式的 Y 轴从上往下。
    // 所以不加处理的话，贴图会上下颠倒（4.2 会讲怎么翻转）。
    //
    // VBO 内存布局（stride = 32 字节）：
    //   |--- 位置(12B) ---|--- 颜色(12B) ---|- 纹理坐标(8B) -|
    //   |  x  |  y  |  z  |  R  |  G  |  B  |  u  |  v     |
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // 右上
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // 右下
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 左下
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // 左上
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

    // 位置属性：3 个 float，stride=32，offset=0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 颜色属性：3 个 float，stride=32，offset=12
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 纹理坐标属性：2 个 float（u, v），stride=32，offset=24
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // ========================================================================
    // 纹理加载流水线
    // 流程：创建纹理对象 → 绑定 → 设置参数 → 加载图片 → 上传 GPU → 生成 mipmap
    // ========================================================================

    // ---- 创建纹理对象 ----
    // glGenTextures(GLsizei n, GLuint *textures)：和 glGenBuffers 一样的模式。
    unsigned int texture;
    glGenTextures(1, &texture);

    // glBindTexture(GLenum target, GLuint texture)：绑定纹理到 GL_TEXTURE_2D 槽位。
    // 之后所有对 GL_TEXTURE_2D 的操作（设置参数、上传数据）都作用在这个纹理上。
    glBindTexture(GL_TEXTURE_2D, texture);

    // ---- 纹理环绕方式（Wrapping）----
    // 纹理坐标超出 [0,1] 范围时怎么办？
    //   GL_REPEAT          — 重复平铺（像地板砖一样不断重复）
    //   GL_MIRRORED_REPEAT — 镜像重复（每块翻转一次）
    //   GL_CLAMP_TO_EDGE   — 拉伸边缘颜色（超出的部分用最近边缘的颜色填）
    //   GL_CLAMP_TO_BORDER — 超出部分用指定纯色填充
    // glTexParameteri(GLenum target, GLenum pname, GLint param)：
    //   target — 哪种纹理（GL_TEXTURE_2D）
    //   pname  — 设置哪个参数（GL_TEXTURE_WRAP_S = X轴环绕, _T = Y轴）
    //   param  — 参数值（GL_REPEAT / GL_CLAMP_TO_EDGE 等）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // ---- 纹理过滤（Filtering）----
    // 当纹理被拉伸或缩小时，屏幕上每个像素该采样哪个纹素（texel，纹理中的一个像素）？
    //
    // 两种情况：
    //   放大 GL_TEXTURE_MAG_FILTER — 物体离相机近，纹理在屏幕上被拉大，一个纹素要覆盖多个屏幕像素（像素数 > 纹素数）。
    //   缩小 GL_TEXTURE_MIN_FILTER — 物体离相机远，纹理在屏幕上被压缩，多个纹素挤进一个屏幕像素（纹素数 > 像素数）。
    //
    // 基础过滤算法（两个参数都能用）：
    //   GL_NEAREST — 最近邻，取离像素中心最近的那一个纹素。快、锐利、有像素马赛克感（Minecraft 风）。
    //   GL_LINEAR  — 双线性插值，取周围 4 个纹素按距离加权平均。慢一点、平滑、略模糊。
    //
    // MAG_FILTER 只能选上面这两个（放大时不需要 mipmap，因为永远在用最高清的原始纹理）。
    //
    // MIN_FILTER 还能用 mipmap 组合（共 6 个可选值）。
    // mipmap = 预先生成的逐级减半分辨率版本（256→128→64→...→1），远处物体用低分辨率版，
    // 既省显存带宽又减少远处闪烁/摩尔纹。
    //
    // 命名规则：GL_[当前层算法]_MIPMAP_[层间算法]
    //   当前层算法 = 在选定的那张 mipmap 里怎么过滤（NEAREST/LINEAR）
    //   层间算法   = 怎么在两张相邻 mipmap 之间混合（NEAREST=选一张、LINEAR=两张混合）
    //
    //   GL_NEAREST_MIPMAP_NEAREST — 选最近的那张 mipmap，里面用最近邻。最快、质量最低。
    //   GL_LINEAR_MIPMAP_NEAREST — 选最近的那张 mipmap，里面用线性。较常用。
    //   GL_NEAREST_MIPMAP_LINEAR — 混合两张相邻 mipmap，里面用最近邻。
    //   GL_LINEAR_MIPMAP_LINEAR  — 混合两张 mipmap，里面也线性（三线性过滤）。质量最高、最慢。← 本例
    //   （外加不带 MIPMAP 的 GL_NEAREST / GL_LINEAR，合计 6 个）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ---- 加载图片 ----
    // stbi_load(const char *filename, int *x, int *y, int *channels, int desired_channels)：
    //   读入图片文件，返回解码后的像素数据（unsigned char*）。内部自动判断 jpg/png 等格式。
    //   x, y        — 图片的宽高，stbi_load 填充
    //   channels    — 图片实际的通道数（RGB=3, RGBA=4），stbi_load 填充
    //   desired_channels — 期望的通道数，0 = 保持原始通道数不变
    //
    // FileSystem::getPath(...)：不是普通的文件名，而是用仓库根路径拼出来的绝对路径。
    // 原理是 CMake 编译时把 ${CMAKE_SOURCE_DIR} 写入 root_directory.h，
    // 然后 getPath() 用这个路径 + 相对路径，保证从任何目录运行都能找到资源。
    int width, height, nrChannels;
    unsigned char *data = stbi_load(FileSystem::getPath("resources/textures/container.jpg").c_str(),&width, &height, &nrChannels, 0);

    if (data)
    {
        // ---- 上传纹理数据到 GPU ----
        // glTexImage2D(GLenum target, GLint level, GLint internalformat,
        //              GLsizei width, GLsizei height, GLint border,
        //              GLenum format, GLenum type, const void *pixels)：
        //   target         — GL_TEXTURE_2D
        //   level          — mipmap 级别，0 = 原始分辨率（最高清）
        //   internalformat — GPU 内部用什么格式存（GL_RGB = 3 通道 8-bit）
        //   width, height  — 图片尺寸
        //   border         — 边框，OpenGL 3.3 只支持 0
        //   format         — 原始数据的格式（GL_RGB = R G B 各一字节）
        //   type           — 每个像素分量的数据类型（GL_UNSIGNED_BYTE）
        //   pixels         — 指向像素数据的指针
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        // glGenerateMipmap(GL_TEXTURE_2D)：自动生成完整 mipmap 链。
        // 之前设了 GL_LINEAR_MIPMAP_LINEAR 但没有实际数据，这行生成各级纹理。
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    // 数据已上传 GPU，CPU 端的副本可以释放了。
    stbi_image_free(data);


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 绑定纹理（因为只有一个纹理对象，绑定一次即可，放循环外也行）
        glBindTexture(GL_TEXTURE_2D, texture);

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
