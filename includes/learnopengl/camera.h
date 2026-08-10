/**
 * Camera 类 — 封装 FPS 风格的飞行相机
 *
 * 管理相机的位置、朝向、移动/视角/缩放，对外提供：
 *   GetViewMatrix()        — 返回 view 矩阵（直接传给 shader）
 *   ProcessKeyboard()      — 处理 WASD 移动
 *   ProcessMouseMovement() — 处理鼠标移动（改变视角）
 *   ProcessMouseScroll()   — 处理滚轮（缩放 fov）
 *
 * 核心数学：用欧拉角（Yaw/Pitch）描述相机朝向，再换算成 Front/Right/Up 三个向量。
 * 这是后面 lighting / model_loading 等所有章节的标准相机，理解后可一直复用。
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 相机移动方向的枚举。用枚举而非直接传按键，是为了和窗口系统解耦
// （GLFW 的按键常量不暴露给 Camera 类）。
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 默认相机参数（构造函数的默认值）
const float YAW         = -90.0f;  // 偏航角：-90° 让相机初始朝 -Z 看（详见下方说明）
const float PITCH       =   0.0f;  // 俯仰角：0° 水平
const float SPEED       =   2.5f;  // 移动速度（单位/秒）
const float SENSITIVITY =   0.1f;  // 鼠标灵敏度
const float ZOOM        =  45.0f;  // 视野角度（fov），也是透视投影的 fovy


class Camera
{
public:
    // ---- 相机属性 ----
    glm::vec3 Position;  // 相机在世界中的位置
    glm::vec3 Front;     // 相机朝向（指向"前方"的单位向量）
    glm::vec3 Up;        // 相机的局部 up（头顶方向）
    glm::vec3 Right;     // 相机的局部 right（右手方向）
    glm::vec3 WorldUp;   // 世界 up（通常固定 (0,1,0)，用于叉乘求 Right）

    // ---- 欧拉角（描述相机朝向）----
    float Yaw;           // 偏航：左右转头（绕世界 Y 轴）
    float Pitch;         // 俯仰：上下点头（绕相机的 Right 轴）

    // ---- 可调参数 ----
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;          // fov，滚轮调节，范围 [1°, 45°]

    // 构造函数（向量版本）：传位置 + up + 欧拉角
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();  // 根据欧拉角算出 Front/Right/Up
    }

    // 构造函数（标量版本）：传各个分量
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 返回 view 矩阵。
    // lookAt(eye, center, up)：
    //   eye    = Position（相机在哪）
    //   center = Position + Front（相机位置 + 前方向量 = 看向的目标点）
    //   up     = Up（相机头顶方向）
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // 处理键盘移动。direction 是枚举，deltaTime 保证移动速度和帧率无关。
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        // velocity = 速度 × 每帧耗时。帧率低时 deltaTime 大 → 单帧位移大 → 总速度一致。
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)  Position += Front * velocity;  // 前
        if (direction == BACKWARD) Position -= Front * velocity;  // 后
        if (direction == LEFT)     Position -= Right * velocity;  // 左
        if (direction == RIGHT)    Position += Right * velocity;  // 右
    }

    // 处理鼠标移动。xoffset/yoffset 是相对上一帧的鼠标位移。
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        // 乘灵敏度，控制鼠标转视角的快慢
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        // 鼠标左右 → Yaw（偏航，左右转头）
        // 鼠标上下 → Pitch（俯仰，上下点头）
        Yaw   += xoffset;
        Pitch += yoffset;

        // 限制 Pitch 在 [-89°, 89°]。
        // 超过 90° 会导致 Front 向量翻转（看向背后），画面瞬间翻转，体验很差。
        if (constrainPitch)
        {
            if (Pitch > 89.0f)  Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }

        // 欧拉角变了，重新算 Front/Right/Up
        updateCameraVectors();
    }

    // 处理滚轮。yoffset 是滚轮的滚动量（+1 / -1）。
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;     // 滚轮上滚 → fov 变小 → 放大（望远镜效果）
        if (Zoom < 1.0f)  Zoom = 1.0f;   // 下限：太小会变成强透视畸变
        if (Zoom > 45.0f) Zoom = 45.0f;  // 上限：太大没意义
    }

private:
    // 根据当前欧拉角（Yaw/Pitch）计算 Front/Right/Up 三个单位向量。
    // 这是欧拉角 → 方向向量的标准三角换算。
    void updateCameraVectors()
    {
        glm::vec3 front;
        // 把 Yaw/Pitch（度）转弧度后代入球坐标公式：
        //   front.x = cos(Yaw)*cos(Pitch)
        //   front.y = sin(Pitch)
        //   front.z = sin(Yaw)*cos(Pitch)
        // 直觉：Yaw 决定在 XZ 平面的朝向（0°朝+X，-90°朝-Z），
        //       Pitch 决定抬头/低头（正=抬头朝+Y）。
        // Yaw 默认 -90° 是因为 0° 时 front.z=sin(0)=0、front.x=cos(0)=1（朝+X），
        // 减到 -90° 让 front.z=sin(-90°)=-1（朝-Z，即默认看进屏幕深处）。
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        // Right = Front × WorldUp（叉乘）：两个向量的叉乘得到垂直于两者的第三个向量。
        //   Front（前）× WorldUp（天上）= Right（右手边）
        // 归一化是因为 Pitch 变化时叉乘结果长度会变，不归一化会导致移动速度漂移。
        Right = glm::normalize(glm::cross(Front, WorldUp));
        // Up = Right × Front：再叉乘一次得到相机的 up（垂直于 Right 和 Front）。
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
#endif
