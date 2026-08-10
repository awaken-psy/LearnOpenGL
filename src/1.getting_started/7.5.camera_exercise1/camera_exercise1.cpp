// 练习1（代码片段，不参与编译）
//
// 焦点：自己手写一个 lookAt 矩阵，理解 glm::lookAt 内部到底做了什么。
//
// lookAt 矩阵 = 旋转矩阵 × 平移矩阵，把世界变换到相机视角：
//   1. 求相机坐标系的三个轴（z=朝后、x=右、y=上）
//   2. 平移矩阵：把世界往相机反方向移（让相机回到原点）
//   3. 旋转矩阵：用三个轴做基变换（旋转世界对齐相机朝向）
//
// 下面是关键代码（用 // 注释掉，仅作参考）：
//
//   // 自定义 lookAt 实现
//   glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp)
//   {
//       // 1. z 轴 = 从目标指向相机的方向（相机看的反方向，因为 OpenGL 相机朝 -Z）
//       glm::vec3 zaxis = glm::normalize(position - target);
//       // 2. x 轴 = worldUp × zaxis（叉乘得垂直于两者的右方向）
//       glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
//       // 3. y 轴 = zaxis × xaxis（相机的上方向）
//       glm::vec3 yaxis = glm::cross(zaxis, xaxis);
//
//       // 平移矩阵：把世界按 -position 移动（等价于相机回到原点）
//       // GLM 是列主序：mat[col][row]，所以 translation[3][0] 是第4列第1行（x 分量）
//       glm::mat4 translation = glm::mat4(1.0f);
//       translation[3][0] = -position.x;
//       translation[3][1] = -position.y;
//       translation[3][2] = -position.z;
//
//       // 旋转矩阵：三个轴作为列向量填入（基变换）
//       glm::mat4 rotation = glm::mat4(1.0f);
//       rotation[0][0] = xaxis.x; rotation[1][0] = xaxis.y; rotation[2][0] = xaxis.z;
//       rotation[0][1] = yaxis.x; rotation[1][1] = yaxis.y; rotation[2][1] = yaxis.z;
//       rotation[0][2] = zaxis.x; rotation[1][2] = zaxis.y; rotation[2][2] = zaxis.z;
//
//       // 最终 view = rotation * translation（从右往左：先平移再旋转）
//       return rotation * translation;
//   }
//
//   // 用自定义版本替换 glm::lookAt：
//   // view = glm::lookAt(eye, center, up);
//   view = calculate_lookAt_matrix(eye, center, up);
