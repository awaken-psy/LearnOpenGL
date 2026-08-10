/**
 * 坐标系统 + 深度测试 — 第一个真正的 3D 立方体
 *
 * 新增内容：
 *   立方体顶点数据   — 36 个顶点（6 面 × 2 三角形 × 3 顶点），不用 EBO
 *   glEnable(GL_DEPTH_TEST)        — 开启深度测试，让前面的面遮挡后面的面
 *   glClear(GL_DEPTH_BUFFER_BIT)   — 每帧清深度缓冲（否则上一帧的深度残留）
 *
 * 和 6.1 的区别：6.1 是单个矩形（6 顶点）。这里是完整的 3D 立方体（36 顶点），
 * 加上深度测试后才能正确显示立体感（否则面会乱叠）。
 *
 * 效果：一个绕斜轴自转的 3D 木箱。
 * 按 ESC 键可以关闭窗口。
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>

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

    // ---- 开启深度测试（3D 渲染必须！）----
    // 没有深度测试：后画的面会无条件盖在先画面上 → 立方体看起来是透明的、面乱叠。
    // 有深度测试：GPU 比较每个像素的深度(z)，近的盖远的 → 正确的遮挡关系。
    glEnable(GL_DEPTH_TEST);

    Shader ourShader("6.2.coordinate_systems.vs", "6.2.coordinate_systems.fs");

    // ---- 立方体顶点：36 个顶点 = 6 面 × 2 三角形 × 3 顶点 ----
    // 为什么不用 EBO？因为每个面的纹理坐标不同（同一顶点在不同面贴图方向不同），
    // 用索引复用顶点反而麻烦，这里直接为每个三角形单独列出顶点（有重复）。
    // 6 个面：后(z=-0.5)、前(z=+0.5)、左(x=-0.5)、右(x=+0.5)、下(y=-0.5)、上(y=+0.5)
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // ---- 加载两张纹理（同 6.1）----
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(FileSystem::getPath("resources/textures/container.jpg").c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    data = stbi_load(FileSystem::getPath("resources/textures/awesomeface.png").c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // 每帧都要清深度缓冲！否则上一帧的深度值残留，导致本帧物体被错误遮挡。
        // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT — 同时清颜色和深度。
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        ourShader.use();

        // ============================================================
        // MVP 矩阵 — 顶点从 3D 局部坐标变换到 2D 屏幕的完整流程
        //
        // 坐标系（右手系）：+Y 上，+X 右，+Z 朝向观察者（出屏），-Z 屏幕深处。
        // 相机默认在原点 (0,0,0)，朝 -Z 看，头顶朝 +Y。
        //
        // 顶点流水线：
        //   局部坐标 →[model]→ 世界坐标 →[view]→ 观察坐标 →[projection]→ 裁剪坐标 → 屏幕
        //   shader 里一行做完：gl_Position = projection * view * model * vec4(aPos, 1.0)
        //   （矩阵从右往左作用：先 model，再 view，最后 projection）
        // ============================================================

        // ---- model 矩阵：把物体放进世界并旋转 ----
        glm::mat4 model = glm::mat4(1.0f);
        // 绕斜轴 (0.5, 1.0, 0.0) 旋转，角度 = glfwGetTime()（随时间增大 → 持续自转）。
        //   轴 (0.5,1,0)：偏向 +X 和 +Y 的斜轴 → 立方体翻滚着转（不是绕单一轴整齐转）。
        //   轴向量不必是单位长度，GLM 内部会自动归一化。
        //   glm::rotate 的角度单位是弧度；glfwGetTime() 返回秒数，正好随时间线性增大。
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));

        // ---- view 矩阵：移动世界来模拟相机后退 ----
        glm::mat4 view = glm::mat4(1.0f);
        // vec3(0,0,-10) 把整个场景沿 -Z 方向推 10 个单位。
        //   等价于相机往 +Z 退 10 → 物体在相机前方 10 单位处（看起来更小，符合透视）。
        //   关键认知：相机永远钉在原点不动，是世界在反向移动来模拟相机移动。
        //   （后面 Camera 章节会用真正的 Camera 类替代这个硬编码平移）
        view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -10.0f));

        // ---- projection 矩阵：透视投影（近大远小），定义一个视锥体 ----
        glm::mat4 projection = glm::mat4(1.0f);
        // perspective(fovy, aspect, near, far) 定义一个截头金字塔（视锥体），
        // 只有落在锥体内的物体才会被渲染：
        //   fovy  = 45°   — 垂直视野角度，≈人眼舒适范围。
        //                   值大 → 广角/鱼眼（视野广、物体显得远）；
        //                   值小 → 长焦/望远镜（视野窄、物体显得近）。
        //   aspect= 800/600 ≈ 1.333 — 窗口宽÷高。必须等于实际窗口比例，
        //                   否则画面被拉伸（圆变椭圆）。窗口改 1920×1080 就得改成 1920/1080。
        //   near  = 0.1   — 近裁剪面距相机的距离。距相机 <0.1 的物体被裁掉（不画）。
        //                   必须严格 >0（透视投影的数学要求）。
        //   far   = 100   — 远裁剪面距相机的距离。距相机 >100 的物体被裁掉。
        //
        // 本例物体被 view 推到 z=-10，距相机 10，满足 0.1 ≤ 10 ≤ 100 → 会被渲染。
        // 经验：near/far 应紧凑包住场景。far 设太大（如 10000）会降低深度缓冲精度，
        //       导致两个相近物体闪来闪去（z-fighting，后面章节会讲）。
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
        unsigned int viewLoc  = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        ourShader.setMat4("projection", projection);

        // 画立方体：36 个顶点（不用 EBO，直接 glDrawArrays）
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

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
