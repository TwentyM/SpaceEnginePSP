#include "HudRenderer.h"

#include <pspgu.h>


namespace
{
    constexpr short ScreenWidth =
        480;

    constexpr short ScreenHeight =
        272;


    constexpr short ScreenCenterX =
        ScreenWidth / 2;

    constexpr short ScreenCenterY =
        ScreenHeight / 2;


    /*
        PSP színek ABGR sorrendűek.
    */

    constexpr unsigned int HudWhite =
        0xFFFFFFFF;

    constexpr unsigned int HudDim =
        0xFF606060;

    constexpr unsigned int HudOrange =
        0xFF40A0FF;


    struct HudVertex
    {
        unsigned int color;

        short x;
        short y;
        short z;
    };
}


void HudRenderer::DrawLine(
    short x1,
    short y1,

    short x2,
    short y2,

    unsigned int color
) const
{
    HudVertex* vertices =
        static_cast<HudVertex*>(
            sceGuGetMemory(
                sizeof(HudVertex) *
                2
            )
        );


    if (vertices == nullptr)
    {
        return;
    }


    vertices[0].color =
        color;

    vertices[0].x =
        x1;

    vertices[0].y =
        y1;

    vertices[0].z =
        0;


    vertices[1].color =
        color;

    vertices[1].x =
        x2;

    vertices[1].y =
        y2;

    vertices[1].z =
        0;


    sceGuDrawArray(
        GU_LINES,

        GU_COLOR_8888 |
        GU_VERTEX_16BIT |
        GU_TRANSFORM_2D,

        2,

        nullptr,
        vertices
    );
}


void HudRenderer::DrawCrosshair() const
{
    constexpr short gap =
        5;

    constexpr short length =
        10;


    // Left
    DrawLine(
        ScreenCenterX -
            gap -
            length,

        ScreenCenterY,

        ScreenCenterX -
            gap,

        ScreenCenterY,

        HudWhite
    );


    // Right
    DrawLine(
        ScreenCenterX +
            gap,

        ScreenCenterY,

        ScreenCenterX +
            gap +
            length,

        ScreenCenterY,

        HudWhite
    );


    // Top
    DrawLine(
        ScreenCenterX,

        ScreenCenterY -
            gap -
            length,

        ScreenCenterX,

        ScreenCenterY -
            gap,

        HudWhite
    );


    // Bottom
    DrawLine(
        ScreenCenterX,

        ScreenCenterY +
            gap,

        ScreenCenterX,

        ScreenCenterY +
            gap +
            length,

        HudWhite
    );
}


void HudRenderer::DrawThrottle(
    float enginePower
) const
{
    if (enginePower < 0.0f)
    {
        enginePower =
            0.0f;
    }


    if (enginePower > 1.0f)
    {
        enginePower =
            1.0f;
    }


    constexpr short x =
        15;

    constexpr short top =
        80;

    constexpr short bottom =
        192;


    DrawLine(
        x,
        top,

        x,
        bottom,

        HudDim
    );


    /*
        Tick marks.
    */
    for (
        int i = 0;
        i <= 4;
        ++i
    )
    {
        const short y =
            static_cast<short>(
                top +
                (
                    bottom -
                    top
                ) *
                i /
                4
            );


        DrawLine(
            x - 3,
            y,

            x + 3,
            y,

            HudDim
        );
    }


    const short markerY =
        static_cast<short>(
            bottom -
            (
                bottom -
                top
            ) *
            enginePower
        );


    DrawLine(
        x - 6,
        markerY,

        x + 6,
        markerY,

        HudOrange
    );


    DrawLine(
        x - 5,
        markerY - 1,

        x + 5,
        markerY - 1,

        HudOrange
    );
}


void HudRenderer::DrawTargetBracket(
    float screenX,
    float screenY,
    float depth
) const
{
    short halfSize =
        18;


    if (depth < 45.0f)
    {
        halfSize =
            25;
    }
    else if (depth > 140.0f)
    {
        halfSize =
            13;
    }


    constexpr short arm =
        7;


    const short x =
        static_cast<short>(
            screenX
        );

    const short y =
        static_cast<short>(
            screenY
        );


    const short left =
        x - halfSize;

    const short right =
        x + halfSize;

    const short top =
        y - halfSize;

    const short bottom =
        y + halfSize;


    // Top-left
    DrawLine(
        left,
        top,

        left + arm,
        top,

        HudOrange
    );

    DrawLine(
        left,
        top,

        left,
        top + arm,

        HudOrange
    );


    // Top-right
    DrawLine(
        right - arm,
        top,

        right,
        top,

        HudOrange
    );

    DrawLine(
        right,
        top,

        right,
        top + arm,

        HudOrange
    );


    // Bottom-left
    DrawLine(
        left,
        bottom,

        left + arm,
        bottom,

        HudOrange
    );

    DrawLine(
        left,
        bottom - arm,

        left,
        bottom,

        HudOrange
    );


    // Bottom-right
    DrawLine(
        right - arm,
        bottom,

        right,
        bottom,

        HudOrange
    );

    DrawLine(
        right,
        bottom - arm,

        right,
        bottom,

        HudOrange
    );


    /*
        Kis target center diamond.
    */
    constexpr short diamond =
        3;


    DrawLine(
        x,
        y - diamond,

        x + diamond,
        y,

        HudOrange
    );

    DrawLine(
        x + diamond,
        y,

        x,
        y + diamond,

        HudOrange
    );

    DrawLine(
        x,
        y + diamond,

        x - diamond,
        y,

        HudOrange
    );

    DrawLine(
        x - diamond,
        y,

        x,
        y - diamond,

        HudOrange
    );
}


void HudRenderer::Draw(
    float enginePower,

    bool targetSelected,
    bool targetOnScreen,

    float targetScreenX,
    float targetScreenY,
    float targetDepth
) const
{
    /*
        Innentől screen-space overlay.
    */
    sceGuDisable(
        GU_LIGHTING
    );

    sceGuDisable(
        GU_LIGHT0
    );

    sceGuDisable(
        GU_TEXTURE_2D
    );

    sceGuDisable(
        GU_DEPTH_TEST
    );

    sceGuDisable(
        GU_BLEND
    );


    DrawCrosshair();


    DrawThrottle(
        enginePower
    );


    if (
        targetSelected &&
        targetOnScreen
    )
    {
        DrawTargetBracket(
            targetScreenX,
            targetScreenY,
            targetDepth
        );
    }


    /*
        Következő frame-re visszaállítjuk.
    */
    sceGuEnable(
        GU_DEPTH_TEST
    );
}