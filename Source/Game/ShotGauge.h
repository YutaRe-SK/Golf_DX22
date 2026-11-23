#pragma once

class ShotGauge
{
public:
    void  Start();
    float Stop();          // その時点の値(0～1)を返してリセット
    void  Update(float deltaTime);

    bool  IsActive() const { return m_active; }
    float GetValue() const { return m_value; }

private:
    float m_value = 0.0f;
    float m_direction = 1.0f;
    bool  m_active = false;

    static constexpr float SPEED = 0.8f;  // 1秒あたりの増減量
};