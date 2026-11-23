#include "Application.h"

bool Application::Initialize(HINSTANCE hInstance)
{
    if (!m_window.Create(hInstance, L"GolfGame", WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        return false;
    }

    if (!m_graphics.Initialize(m_window.GetHandle(), m_window.GetWidth(), m_window.GetHeight()))
    {
        return false;
    }

    if (!m_game.Initialize(&m_graphics))
    {
        return false;
    }

    m_timer.Initialize();
    return true;
}

int Application::Run()
{
    while (m_window.ProcessMessages())
    {
        if (m_window.ConsumeResize())
        {
            m_graphics.Resize(m_window.GetWidth(), m_window.GetHeight());
            m_game.OnResize(m_window.GetWidth(), m_window.GetHeight());
        }

        if (m_window.IsMinimized())
        {
            Sleep(16);
            continue;
        }

        float dt = m_timer.Tick();
        m_input.Update(m_window.GetHandle());
        Update(dt);
        Render();
    }
    return 0;
}

void Application::Update(float deltaTime)
{
    m_game.Update(deltaTime, m_input);
    UpdateTitle();
}

void Application::UpdateTitle()
{
    // クラブ名は半角英数字なので、1文字ずつwchar_tへ変換する
    std::wstring title = L"GolfGame  Club: ";
    for (const char* p = m_game.GetClubName(); *p; ++p)
    {
        title += static_cast<wchar_t>(*p);
    }
    title += L"  Strokes: " + std::to_wstring(m_game.GetStrokes());

    // 変化したときだけ更新する
    if (title != m_lastTitle)
    {
        SetWindowTextW(m_window.GetHandle(), title.c_str());
        m_lastTitle = title;
    }
}

void Application::Render()
{
    m_graphics.BeginFrame(0.53f, 0.81f, 0.92f, 1.0f);
    m_game.Render();
    m_graphics.EndFrame();
}

void Application::Shutdown()
{
    m_graphics.Shutdown();
    m_window.Destroy();
}