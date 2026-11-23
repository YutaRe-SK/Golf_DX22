#include "Input.h"
#include <cstring>

void Input::Update(HWND hWnd)
{
    std::memcpy(m_previous, m_current, sizeof(m_current));

    bool focused = (GetForegroundWindow() == hWnd);
    for (int i = 0; i < 256; ++i)
    {
        m_current[i] = focused && (GetAsyncKeyState(i) & 0x8000) != 0;
    }
}

bool Input::IsDown(int vk) const
{
    return (vk >= 0 && vk < 256) ? m_current[vk] : false;
}

bool Input::IsTriggered(int vk) const
{
    return (vk >= 0 && vk < 256) ? (m_current[vk] && !m_previous[vk]) : false;
}