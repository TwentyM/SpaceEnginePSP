#pragma once

#include "../Graphics/Renderer.h"
#include "../Input/Input.h"
#include "../../Game/Game.h"

class Application
{
public:
    Application();
    ~Application();

    void Run();

private:
    bool Initialize();
    void Shutdown();

private:
    Renderer m_renderer;
    Input m_input;
    Game m_game;
};