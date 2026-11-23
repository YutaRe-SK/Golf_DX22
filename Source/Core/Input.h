#pragma once
#include <Windows.h>

class Input
{
public:
    // 毎フレーム1回呼ぶ。ウィンドウが非アクティブの間は入力を無視する
    void Update(HWND hWnd);

    bool IsDown(int vk) const;       // 押している間ずっとtrue
    bool IsTriggered(int vk) const;  // 押した瞬間だけtrue

private:
    bool m_current[256] = {};
    bool m_previous[256] = {};
};