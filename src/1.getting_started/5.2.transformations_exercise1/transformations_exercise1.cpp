// 练习1（代码片段，不参与编译）
//
// 焦点：把 5.1 里 translate 和 rotate 的调用顺序对调，观察效果变化。
//
// 5.1 原版（先 translate 后 rotate）：
//   transform = translate(...)      → T
//   transform = rotate(transform)   → T * R
//   作用顶点：T * R * v = T * (R * v)  → 先自转，再平移 → 右下角原地自转
//
// 本练习（先 rotate 后 translate）：
//   transform = rotate(...)         → R
//   transform = translate(transform)→ R * T
//   作用顶点：R * T * v = R * (T * v)  → 先平移到右下，再绕原点旋转 → 绕屏幕中心公转
//
// 一行代码顺序的调换，效果完全不同——这就是矩阵乘法不可交换的直观体现。

#if 0 // 教学片段（含 [...] 占位、不完整），用 #if 0 屏蔽避免 IntelliSense 报错
int main()
{
    [...]
    while(!glfwWindowShouldClose(window))
    {
        [...]
        // create transformations
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); // 调换了顺序：先 rotate
        transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));                   // 调换了顺序：后 translate
        [...]
    }
}
#endif

/*
答案翻译：为什么这次容器会绕着屏幕转圈？
====================================================

记住：矩阵乘法是反向应用的（从右往左作用于顶点）。
这次（transform = R * T），平移反而先作用于容器，把它放到屏幕右下角；
平移之后，旋转才作用到这个已经平移过的容器上。

旋转本质上是一种"基变换"（换坐标系）。因为我们改变了容器的基，
后续的平移会基于新的基向量来移动——容器一旦稍微转了一下，
原本的"向右平移"就会变成沿旋转后的方向平移。

如果先旋转，旋转是绕原点 (0,0,0) 进行的；但因为容器先被平移了，
它的旋转中心不再是 (0,0,0)，看起来就像绕着场景原点在画圈公转。

如果觉得难以想象，别担心。多动手实验几次变换就能建立起直觉，
这需要练习和经验积累。
*/
