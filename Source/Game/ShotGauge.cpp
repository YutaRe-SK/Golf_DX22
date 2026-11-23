#include "ShotGauge.h"

void ShotGauge::Start()
{
    m_value = 0.0f;
    m_direction = 1.0f;
    m_active = true;
}

float ShotGauge::Stop()
{
    float v = m_value;
    m_value = 0.0f;
    m_active = false;
    return v;
}

void ShotGauge::Update(float deltaTime)
{
    if (!m_active) return;

    // 0¨1¨0¨1c‚Æ‰•œ‚·‚é
    m_value += m_direction * SPEED * deltaTime;
    if (m_value >= 1.0f)
    {
        m_value = 1.0f;
        m_direction = -1.0f;
    }
    else if (m_value <= 0.0f)
    {
        m_value = 0.0f;
        m_direction = 1.0f;
    }
}