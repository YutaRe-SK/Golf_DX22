#pragma once
#include <Windows.h>

class Window
{
public:
    bool Create(HINSTANCE hInstance, const wchar_t* title, int width, int height);
    void Destroy();

    // falseが返ったらアプリ終了
    bool ProcessMessages();

    // サイズ変更があったら一度だけtrueを返す
    bool ConsumeResize();

    HWND GetHandle() const { return m_hWnd; }
    int  GetWidth()  const { return m_width; }
    int  GetHeight() const { return m_height; }
    bool IsMinimized() const { return m_minimized; }

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    HWND      m_hWnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    int  m_width = 0;
    int  m_height = 0;
    bool m_resized = false;
    bool m_minimized = false;

    static constexpr const wchar_t* CLASS_NAME = L"GolfGameWindowClass";
};
