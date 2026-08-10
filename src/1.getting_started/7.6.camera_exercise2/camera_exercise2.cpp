// 练习2（代码片段，不参与编译）
//
// 焦点：FPS 风格相机 —— 强制相机贴地（y 固定为 0），不能飞行。
//
// 做法：在 Camera 类的 ProcessKeyboard 末尾加一行 Position.y = 0.0f，
//       无论怎么移动都把 y 分量清零，相机就被锁在 XZ 平面上。
//
// 这是第一人称射击游戏（FPS）常见的"贴地行走"处理。
//
// 下面是 Camera::ProcessKeyboard 的修改版（用 // 注释掉，仅作参考）：
//
//   void ProcessKeyboard(Camera_Movement direction, float deltaTime)
//   {
//       float velocity = MovementSpeed * deltaTime;
//       if (direction == FORWARD)  Position += Front * velocity;
//       if (direction == BACKWARD) Position -= Front * velocity;
//       if (direction == LEFT)     Position -= Right * velocity;
//       if (direction == RIGHT)    Position += Right * velocity;
//
//       // ← 新增这一行：强制 y=0，把相机钉在地面上（XZ 平面）
//       Position.y = 0.0f;
//   }
//
// 注意：这会让前后移动也"贴地"——因为 Front 向量在抬头/低头时会带 y 分量，
// 但清零 y 后，实际位移只在 XZ 平面，符合 FPS 走路的感觉。
