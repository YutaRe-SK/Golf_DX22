#include "Ball.h"
#include <cmath>
#include <algorithm>

namespace
{
    constexpr float GRAVITY         = 9.8f;   // 重力加速度 (m/s^2)
    constexpr float AIR_DRAG        = 0.05f;  // 空気抵抗（1秒あたりの減速率）
    constexpr float BOUNCE          = 0.35f;  // 着地時の縦方向の反発係数
    constexpr float IMPACT_FRICTION = 0.75f;  // 着地時に残る横方向の速度の割合
    constexpr float MIN_BOUNCE_VY   = 1.0f;   // これより遅い落下ならバウンドせず転がりに移る
    constexpr float ROLL_DECEL      = 2.0f;   // 転がりの減速 (m/s^2)
    constexpr float STOP_SPEED      = 0.05f;  // この速度以下で停止
    constexpr float SUB_STEP        = 1.0f / 240.0f; // 物理計算の刻み
}

void Ball::Reset(const DirectX::XMFLOAT3& position)
{
    m_position = position;
    m_velocity = { 0.0f, 0.0f, 0.0f };
    m_moving = false;
}

void Ball::Hit(float yaw, float launchAngle, float speed)
{
    float horizontal = cosf(launchAngle) * speed;
    m_velocity.x = sinf(yaw) * horizontal;
    m_velocity.y = sinf(launchAngle) * speed;
    m_velocity.z = cosf(yaw) * horizontal;
    m_moving = true;
}

void Ball::Update(float deltaTime)
{
    if (!m_moving) return;

    // 速い球が地面をすり抜けないよう、細かく分けて計算する
    int steps = static_cast<int>(ceilf(deltaTime / SUB_STEP));
    if (steps < 1) steps = 1;
    float dt = deltaTime / steps;

    for (int i = 0; i < steps && m_moving; ++i)
    {
        Step(dt);
    }
}

void Ball::Step(float dt)
{
    bool onGround = (m_position.y <= RADIUS + 0.001f) && (m_velocity.y <= 0.0f);

    if (!onGround)
    {
        // 空中
        m_velocity.y -= GRAVITY * dt;

        float drag = 1.0f - AIR_DRAG * dt;
        m_velocity.x *= drag;
        m_velocity.y *= drag;
        m_velocity.z *= drag;

        m_position.x += m_velocity.x * dt;
        m_position.y += m_velocity.y * dt;
        m_position.z += m_velocity.z * dt;

        // 着地
        if (m_position.y < RADIUS)
        {
            m_position.y = RADIUS;
            if (m_velocity.y < -MIN_BOUNCE_VY)
            {
                m_velocity.y = -m_velocity.y * BOUNCE;
                m_velocity.x *= IMPACT_FRICTION;
                m_velocity.z *= IMPACT_FRICTION;
            }
            else
            {
                m_velocity.y = 0.0f;
            }
        }
    }
    else
    {
        // 転がり
        m_position.y = RADIUS;
        m_velocity.y = 0.0f;

        float speed = sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
        if (speed < STOP_SPEED)
        {
            m_velocity = { 0.0f, 0.0f, 0.0f };
            m_moving = false;
            return;
        }

        float newSpeed = (std::max)(0.0f, speed - ROLL_DECEL * dt);
        float scale = newSpeed / speed;
        m_velocity.x *= scale;
        m_velocity.z *= scale;

        m_position.x += m_velocity.x * dt;
        m_position.z += m_velocity.z * dt;
    }
}