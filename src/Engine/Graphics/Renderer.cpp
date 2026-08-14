#include "Renderer.h"

#include <pspdisplay.h>
#include <pspgu.h>

namespace
{
    unsigned int __attribute__((aligned(16)))
        g_displayList[262144];
}

bool Renderer::Initialize()
{
    void* frameBuffer0 =
        guGetStaticVramBuffer(
            BufferWidth,
            ScreenHeight,
            GU_PSM_8888
        );

    void* frameBuffer1 =
        guGetStaticVramBuffer(
            BufferWidth,
            ScreenHeight,
            GU_PSM_8888
        );

    void* depthBuffer =
        guGetStaticVramBuffer(
            BufferWidth,
            ScreenHeight,
            GU_PSM_4444
        );

    sceGuInit();

    sceGuStart(
        GU_DIRECT,
        g_displayList
    );

    sceGuDrawBuffer(
        GU_PSM_8888,
        frameBuffer0,
        BufferWidth
    );

    sceGuDispBuffer(
        ScreenWidth,
        ScreenHeight,
        frameBuffer1,
        BufferWidth
    );

    sceGuDepthBuffer(
        depthBuffer,
        BufferWidth
    );

    sceGuOffset(
        2048 - (ScreenWidth / 2),
        2048 - (ScreenHeight / 2)
    );

    sceGuViewport(
        2048,
        2048,
        ScreenWidth,
        ScreenHeight
    );

    sceGuDepthRange(
        65535,
        0
    );

    sceGuScissor(
        0,
        0,
        ScreenWidth,
        ScreenHeight
    );

    sceGuEnable(GU_SCISSOR_TEST);

    sceGuDepthFunc(GU_GEQUAL);
    sceGuEnable(GU_DEPTH_TEST);

    sceGuShadeModel(GU_SMOOTH);

    sceGuDisable(GU_TEXTURE_2D);
    sceGuDisable(GU_CULL_FACE);

    sceGuFinish();

    sceGuSync(
        GU_SYNC_FINISH,
        GU_SYNC_WHAT_DONE
    );

    sceDisplayWaitVblankStart();

    sceGuDisplay(GU_TRUE);

    return true;
}

void Renderer::Shutdown()
{
    sceGuDisplay(GU_FALSE);
    sceGuTerm();
}

void Renderer::BeginFrame()
{
    sceGuStart(
        GU_DIRECT,
        g_displayList
    );

    // Közel fekete, enyhén kékes háttér.
    sceGuClearColor(0xFF100D0A);
    sceGuClearDepth(0);

    sceGuClear(
        GU_COLOR_BUFFER_BIT |
        GU_DEPTH_BUFFER_BIT
    );
}

void Renderer::EndFrame()
{
    sceGuFinish();

    sceGuSync(
        GU_SYNC_FINISH,
        GU_SYNC_WHAT_DONE
    );

    sceDisplayWaitVblankStart();

    sceGuSwapBuffers();
}