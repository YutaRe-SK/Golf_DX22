#pragma once
#include <Windows.h>
#include <string>
#include "Window.h"
#include "Timer.h"
#include "Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Game/Game.h"

class Application
{
public:
    bool Initialize(HINSTANCE hInstance);
    int  Run();
    void Shutdown();

private:
    void Update(float deltaTime);
    void Render();
    void UpdateTitle();

    Window         m_window;
    Timer          m_timer;
    Input          m_input;
    GraphicsDevice m_graphics;
    Game           m_game;

    std::wstring m_lastTitle;

    static constexpr int WINDOW_WIDTH = 1280;
    static constexpr int WINDOW_HEIGHT = 720;
};