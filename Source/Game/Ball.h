#pragma once
#include <DirectXMath.h>

class Ball
{
public:
    void Reset(const DirectX::XMFLOAT3& position);

    // yaw: 水平方向の角度(ラジアン, 0で+Z方向)  launchAngle: 打ち出し角(ラジアン)  speed: 初速(m/s)
    void Hit(float yaw, float launchAngle, float speed);
    void Update(float deltaTime);

    bool IsMoving() const { return m_moving; }
    const DirectX::XMFLOAT3& GetPosition() const { return m_position; }

    // 実物は0.021mだが、見やすさ優先で大きめにしている
    static constexpr float RADIUS = 0.1f;

private:
    void Step(float dt);

    DirectX::XMFLOAT3 m_position = { 0.0f, RADIUS, 0.0f };
    DirectX::XMFLOAT3 m_velocity = { 0.0f, 0.0f, 0.0f };
    bool m_moving = false;
};