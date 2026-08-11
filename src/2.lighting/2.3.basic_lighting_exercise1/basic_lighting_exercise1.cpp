/**
 * 练习 2.3 — 让光源动起来
 *
 * 目标:在渲染循环里用 sin(glfwGetTime()) 让光源位置随时间移动,观察光影实时变化,
 * 直观感受"光照方向改变 → 物体明暗分布改变"。
 *
 * 下面是教程给出的【核心片段】(不是完整程序),只展示要加进渲染循环开头的两行。
 * 要运行:把这两行加进 2.1 或 2.2 的完整 cpp 的渲染循环里(在 setVec3("lightPos") 之前)。
 *
 *   lightPos.x = 1.0 + sin(time) * 2.0;   // x 在 [-1, 3] 之间摆动
 *   lightPos.y = sin(time / 2) * 1.0;     // y 摆得更慢,形成椭圆轨迹
 *
 * ⚠ 代码不完整([... 代表省略部分),用 #if 0 屏蔽以免 IntelliSense 报错。
 */

#if 0
int main()
{
    [...]
    // render loop
    while(!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // clear the colorbuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // change the light's position values over time (can be done anywhere in the render loop actually, but try to do it at least before using the light source positions)
        lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
        lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;

        // set uniforms, draw objects
        [...]

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
#endif
