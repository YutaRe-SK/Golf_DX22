#include "Timer.h"

void Timer::Initialize()
{
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_prev);
    m_deltaTime = 0.0f;
    m_totalTime = 0.0f;
}

float Timer::Tick()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    m_deltaTime = static_cast<float>(now.QuadPart - m_prev.QuadPart)
        / static_cast<float>(m_frequency.QuadPart);
    m_prev = now;

    // ブレークポイント停止やウィンドウのドラッグ後に、ボールが吹っ飛ばないよう上限を設ける
    if (m_deltaTime > 0.1f)
    {
        m_deltaTime = 0.1f;
    }

    m_totalTime += m_deltaTime;
    return m_deltaTime;
}