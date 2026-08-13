/**
 * 模板测试（Stencil Testing）—— 物体轮廓/描边效果
 *
 * 本演示利用【模板缓冲】(Stencil Buffer) 实现物体轮廓描边效果。
 * 模板测试允许我们控制哪些像素可以被绘制，是一种像素级的遮罩机制。
 *
 * 核心概念：
 * - 【模板缓冲】：为每个像素存储一个整数值，可用于控制是否接受或拒绝片段
 * - 【模板函数】glStencilFunc：设置模板测试的比较方式（比较值、参考值、掩码）
 * - 【模板操作】glStencilOp：设置模板测试通过/失败时如何更新模板缓冲
 * - 【模板掩码】glStencilMask：控制哪些位可以被写入模板缓冲（0x00=禁止写入，0xFF=允许写入）
 *
 * 描边实现原理（两遍渲染）：
 * 1. 第一遍：正常绘制物体，同时将物体覆盖的像素在模板缓冲中写入 1
 * 2. 第二遍：绘制放大版的物体，但只在模板值 != 1 的地方绘制（即物体边缘）
 *    这样就只绘制了放大部分与原物体的差值——即轮廓边框
 *
 * 与之前 demo 的区别：
 * - 新增了模板测试相关配置
 * - 使用两个 shader：一个正常渲染，一个输出纯色用于描边
 * - 地板不写入模板缓冲（glStencilMask(0x00)），只有箱子参与轮廓计算
 * 
 *   简单说：

   - `glStencilFunc` 的 mask = 读掩码，决定比较时看哪些位
   - `glStencilMask` 的 mask = 写掩码，决定更新时能改哪些位

  在本 demo 中：
   - 地板：glStencilMask(0x00) → 禁止写模板 → 地板不影响模板缓冲
   - 箱子：glStencilMask(0xFF) → 允许写模板 → glStencilOp 的 REPLACE 生效，模板值变成 1
   - 描边：glStencilMask(0x00) → 禁止写模板 → 描边渲染不破坏已有模板值
 */
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ---- 深度测试 + 模板测试配置 ----
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    // ⭐ glStencilOp — 定义模板缓冲在三种情况下的更新方式
    //
    //   每个片段渲染时会经历两道测试：先模板测试，后深度测试。
    //   三个参数对应三种结果：
    //
    //   参数1: sfail   — 模板测试失败时，怎么处理模板值
    //   参数2: dpfail  — 模板测试通过、但深度测试失败时，怎么处理模板值
    //   参数3: dppass  — 模板测试和深度测试都通过时，怎么处理模板值
    //
    //   GL_KEEP    = 保持模板值不变（不写入）
    //   GL_REPLACE = 用 glStencilFunc 的参考值替换模板值
    //
    //   这里设为 (GL_KEEP, GL_KEEP, GL_REPLACE) 的含义：
    //     - 模板测试失败 → 不动模板值
    //     - 模板通过了但被深度挡住 → 不动模板值
    //     - 模板和深度都通过 → 把模板值改成参考值(后面 glStencilFunc 设的 1)
    //
    //   ⭐ 这行是全局配置，三步渲染都受影响，但配合每步不同的 glStencilFunc 和 glStencilMask 实现不同效果。
    //     - 第零步(地板): glStencilMask(0x00) 禁止写模板 → 模板值保持 0
    //     - 第一步(箱子): glStencilFunc(GL_ALWAYS, 1, 0xFF) + glStencilMask(0xFF)
    //                     → GL_ALWAYS 永远通过 → dppass 生效 → 模板值变成 1
    //     - 第二步(描边): glStencilFunc(GL_NOTEQUAL, 1, 0xFF) + glStencilMask(0x00)
    //                     → 模板值=1 的像素(箱子本体)测试失败 → 不画
    //                     → 模板值=0 的像素(箱子边缘)测试通过 → 画出来 = 描边
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // ⭐ glStencilFunc — 定义模板测试的比较规则
    //
    //   每个片段渲染前，GPU 会执行比较：
    //     (模板缓冲中的当前值 & 掩码)  比较  (参考值 & 掩码)
    //
    //   参数1: func     — 比较函数
    //   参数2: ref      — 参考值（整数）
    //   参数3: mask     — 掩码（按位与，0xFF = 全部位参与比较，0x00 = 永远相等）
    //
    //   这里设为 (GL_NOTEQUAL, 1, 0xFF) 的含义：
    //     比较规则 = 模板值 != 1
    //     - 模板值 = 1（箱子本体的像素）→ 测试失败 → 片段被丢弃，不绘制
    //     - 模板值 = 0（箱子边缘/背景）  → 测试通过 → 片段被绘制
    //
    //   ⭐ 配合第二步渲染放大的箱子：放大后超出原箱子的部分落在模板值=0的区域，
    //     通过测试被画出来；原箱子覆盖的区域模板值=1，测试失败不画。
    //     最终只画出"多出来的边缘" = 描边效果。
    //
    //   注意：这一行虽然在循环外设了一次，但第一步渲染前会改成 GL_ALWAYS，
    //         第二步渲染前再改回 GL_NOTEQUAL。这里预设 NOTEQUAL 是初始状态。
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    // build and compile shaders
    // -------------------------
    Shader shader("2.stencil_testing.vs", "2.stencil_testing.fs");
    // ⭐ 第二个 shader 用于绘制纯色描边，共享同一个顶点着色器
    Shader shaderSingleColor("2.stencil_testing.vs", "2.stencil_single_color.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
        // positions          // texture Coords
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
    float planeVertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // load textures
    // -------------
    unsigned int cubeTexture = loadTexture(FileSystem::getPath("resources/textures/marble.jpg").c_str());
    unsigned int floorTexture = loadTexture(FileSystem::getPath("resources/textures/metal.png").c_str());

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("texture1", 0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        // ⭐ 每帧都要清除模板缓冲（GL_STENCIL_BUFFER_BIT）
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!

        // set uniforms
        shaderSingleColor.use();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shaderSingleColor.setMat4("view", view);
        shaderSingleColor.setMat4("projection", projection);

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // ---- 第零步：绘制地板（不写入模板缓冲）----
        // ⭐ glStencilMask(0x00) 禁止写入模板缓冲，这样地板不会影响后续的描边逻辑
        // 只有箱子参与模板测试，地板只作为背景正常渲染
        glStencilMask(0x00);
        // floor
        glBindVertexArray(planeVAO);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // ---- 第一步：正常绘制箱子，同时写入模板缓冲 ----
        // ⭐ glStencilFunc(GL_ALWAYS, 1, 0xFF)：所有片段都通过模板测试
        //   glStencilMask(0xFF)：允许写入模板缓冲
        //   结合之前设置的 glStencilOp(...GL_REPLACE)，箱子覆盖的像素模板值将被设为 1
        // --------------------------------------------------------------------
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        // cubes
        glBindVertexArray(cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ---- 第二步：绘制放大版箱子作为描边 ----
        // ⭐ 关键逻辑：
        //   1. glStencilFunc(GL_NOTEQUAL, 1, 0xFF)：只在模板值 != 1 的像素上绘制
        //   2. glStencilMask(0x00)：不再写入模板缓冲，保护第一步的结果
        //   3. glDisable(GL_DEPTH_TEST)：放大版箱子不会被自身遮挡
        //   4. 放大版箱子比原始箱子大 10%，多出来的部分就是描边
        //   因为原始箱子覆盖的区域模板值=1，所以放大版在这些区域被模板测试拒绝，
        //   只有超出原始箱子边界的部分才会通过——形成轮廓效果
        // -----------------------------------------------------------------------------------------------------------------------------
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        shaderSingleColor.use();
        float scale = 1.1f;
        // cubes
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        model = glm::scale(model, glm::vec3(scale, scale, scale));
        shaderSingleColor.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale, scale, scale));
        shaderSingleColor.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        // ⭐ 恢复模板测试和深度测试的默认状态，避免影响下一帧
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
