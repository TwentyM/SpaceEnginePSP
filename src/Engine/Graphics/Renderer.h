#pragma once

class Renderer
{
public:
    bool Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

public:
    static constexpr int BufferWidth  = 512;
    static constexpr int ScreenWidth  = 480;
    static constexpr int ScreenHeight = 272;
};