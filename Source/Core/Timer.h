#pragma once
#include <Windows.h>

class Timer
{
public:
    void  Initialize();
    float Tick();                    // ‘O‰ñ‚©‚ç‚ÌŒo‰ß•b‚ð•Ô‚·
    float GetDeltaTime() const { return m_deltaTime; }
    float GetTotalTime() const { return m_totalTime; }

private:
    LARGE_INTEGER m_frequency = {};
    LARGE_INTEGER m_prev = {};
    float m_deltaTime = 0.0f;
    float m_totalTime = 0.0f;
};