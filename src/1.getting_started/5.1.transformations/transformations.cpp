/**
 * 变换（Transformations）— 用矩阵让物体动起来
 *
 * 新增内容：
 *   GLM 数学库       — OpenGL Mathematics，提供 vec3/mat4 等类型和矩阵运算
 *   glm::mat4        — 4×4 矩阵，能同时表达平移/旋转/缩放
 *   glm::translate   — 生成平移矩阵
 *   glm::rotate      — 生成旋转矩阵
 *   glUniformMatrix4fv — 把 4×4 矩阵传给 shader 的 uniform
 *   glm::value_ptr   — 把 GLM 矩阵转成 OpenGL 需要的裸指针
 *
 * 效果：矩形在右下方随时间自转。
 * 按 ESC 键可以关闭窗口。
 *
 * 为什么用矩阵？
 *   不用矩阵：要手写每个顶点的新坐标，物体一旋转/平移就得重算所有顶点。
 *   用矩阵：一个 4×4 矩阵描述"怎么变换"，shader 里 matrix * pos 一行搞定。
 *   而且多个变换可以预先乘成一个矩阵（矩阵乘法的结合律），GPU 只需做一次乘法。
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

// GLM 三个常用头文件：
//   glm.hpp             — 核心类型 vec2/vec3/mat4 等
//   matrix_transform.hpp — translate/rotate/scale 等变换函数
//   type_ptr.hpp        — value_ptr()，把 GLM 类型转成 OpenGL 接受的指针
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

    Shader ourShader("5.1.transform.vs", "5.1.transform.fs");

    // ---- 顶点数据：位置(3) + 纹理坐标(2)，去掉了颜色属性 ----
    // 现在用纹理贴图，不需要顶点颜色了，stride 从 8 降到 5。
    float vertices[] = {
        // positions           // texture coords
         0.5f,  0.5f, 0.0f,    1.0f, 1.0f, // 右上
         0.5f, -0.5f, 0.0f,    1.0f, 0.0f, // 右下
        -0.5f, -0.5f, 0.0f,    0.0f, 0.0f, // 左下
        -0.5f,  0.5f, 0.0f,    0.0f, 1.0f  // 左上
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

    // 位置属性：stride = 5 * sizeof(float)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标属性：offset = 3 * sizeof(float)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // ---- 加载两张纹理（同 4.2）----
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

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // ====================================================================
        // 构造变换矩阵（每帧重新算，因为旋转角度随时间变化）
        // ====================================================================

        // glm::mat4(1.0f) — 单位矩阵（对角线为 1，其余为 0）。
        // 任何向量 × 单位矩阵 = 向量本身。它是矩阵变换的"起点"（类比数值 1）。
        // 一定要初始化！不初始化的话矩阵里是垃圾内存，变换结果完全乱套。
        glm::mat4 transform = glm::mat4(1.0f);

        // glm::translate(mat, vec3) — 在 mat 基础上叠加一个平移。
        //   vec3(0.5, -0.5, 0.0) = 向右 0.5、向下 0.5（屏幕坐标系 Y 朝上，所以 -0.5 是向下）
        // 返回新矩阵，原 mat 不变（所以要 transform = ... 接住返回值）。
        transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));

        // glm::rotate(mat, angle, axis) — 在 mat 基础上叠加一个旋转。
        //   angle = (float)glfwGetTime() — 弧度！不是度数。随时间增大 → 持续旋转。
        //   axis = vec3(0,0,1) — 绕 Z 轴旋转。Z 轴垂直屏幕指向你，
        //          绕它转就是在屏幕平面内转（2D 旋转）。
        transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

        // ⚠️ 变换顺序（最容易踩的坑）：
        //   代码书写顺序：先 translate，后 rotate
        //   实际作用于顶点：先 rotate，后 translate（反过来！）
        //   原因：GLM 的 translate/rotate 是右乘，transform = T * R。
        //         作用于顶点 v：T * R * v = T * (R * v)，右边(R)先算。
        //   记忆口诀：代码从上往下写，顶点从最后一个变换开始经历。
        //   想让物体"先缩放再旋转再平移"，代码就要反过来写：平移→旋转→缩放。

        // ---- 把矩阵传给 shader ----
        ourShader.use();
        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");

        // glUniformMatrix4fv(location, count, transpose, value)：
        //   location  — uniform 的位置
        //   count     — 传几个矩阵（1）
        //   transpose — 是否转置。GL_FALSE = 不转置（GLM 默认主序和 OpenGL 一致）
        //   value     — 指向矩阵首元素的指针
        // glm::value_ptr(mat) 把 GLM 的 mat4 转成 const float*（16 个连续 float）。
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

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
