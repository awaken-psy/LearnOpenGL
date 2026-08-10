// 练习3（代码片段，不参与编译）
//
// 焦点：在 6.3 的 10 个立方体循环里，让每 3 个立方体用时间驱动旋转，其余保持固定角度。
//
// 和 6.3 的区别：6.3 所有立方体都用固定的 angle = 20°*i 旋转（静态）。
// 这里给每 3 个（i%3==0）换成 glfwGetTime()*25°，让它们持续转起来。
//
// 下面是关键代码片段（每行用 // 注释掉，仅作参考，不编译）：
//
//   ...
//
//   glBindVertexArray(VAO);
//   for(unsigned int i = 0; i < 10; i++)
//   {
//       // 每个立方体单独算一个 model 矩阵（位置 + 旋转）
//       glm::mat4 model = glm::mat4(1.0f);
//       model = glm::translate(model, cubePositions[i]);
//       float angle = 20.0f * i;
//       // 每 3 个立方体（第 0、3、6、9 个）用 GLFW 时间做角度 → 持续旋转
//       // 其余的用固定角度 → 静态倾斜
//       if(i % 3 == 0)
//           angle = glfwGetTime() * 25.0f;   // 25 是旋转速度系数（度/秒）
//       model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
//       ourShader.setMat4("model", model);
//
//       glDrawArrays(GL_TRIANGLES, 0, 36);
//   }
//
//   ...
