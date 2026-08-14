#include "Application.h"

#include <pspkernel.h>

namespace
{
    volatile bool g_running = true;

    int ExitCallback(
        int arg1,
        int arg2,
        void* common
    )
    {
        (void)arg1;
        (void)arg2;
        (void)common;

        g_running = false;

        return 0;
    }

    int CallbackThread(
        SceSize args,
        void* argp
    )
    {
        (void)args;
        (void)argp;

        const int callbackId =
            sceKernelCreateCallback(
                "Exit Callback",
                ExitCallback,
                nullptr
            );

        sceKernelRegisterExitCallback(
            callbackId
        );

        sceKernelSleepThreadCB();

        return 0;
    }

    void SetupCallbacks()
    {
        const int threadId =
            sceKernelCreateThread(
                "Callback Thread",
                CallbackThread,
                0x11,
                0xFA0,
                0,
                nullptr
            );

        if (threadId >= 0)
        {
            sceKernelStartThread(
                threadId,
                0,
                nullptr
            );
        }
    }
}

Application::Application()
{
}

Application::~Application()
{
}

bool Application::Initialize()
{
    SetupCallbacks();

    if (!m_renderer.Initialize())
    {
        return false;
    }

    m_input.Initialize();
    m_game.Initialize();

    return true;
}

void Application::Run()
{
    if (!Initialize())
    {
        return;
    }


    // Egyelőre VSync-hez kötött 60 Hz-es prototípus.
    // Később rendes Time/DeltaTime osztályra cseréljük.
    constexpr float deltaTime =
        1.0f / 60.0f;


    while (g_running)
    {
        m_input.Update();

        m_game.Update(
            m_input,
            deltaTime
        );

        m_renderer.BeginFrame();

        m_game.Render(
            m_renderer
        );

        m_renderer.EndFrame();
    }


    Shutdown();

    sceKernelExitGame();
}

void Application::Shutdown()
{
    m_renderer.Shutdown();
}