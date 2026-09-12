#include "HudRenderer.h"

#include <pspgu.h>

#include <cmath>


namespace
{
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


    vertices[0] =
    {
        color,
        x1,
        y1,
        0
    };


    vertices[1] =
    {
        color,
        x2,
        y2,
        0
    };


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


void HudRenderer::DrawThrottle(
    float throttle
) const
{
    if (throttle > 1.0f)
    {
        throttle =
            1.0f;
    }


    if (throttle < -1.0f)
    {
        throttle =
            -1.0f;
    }


    constexpr short x =
        15;

    constexpr short top =
        76;

    constexpr short center =
        136;

    constexpr short bottom =
        196;


    DrawLine(
        x,
        top,

        x,
        bottom,

        HudDim
    );


    // +100 %
    DrawLine(
        x - 3,
        top,

        x + 3,
        top,

        HudDim
    );


    // +50 %
    DrawLine(
        x - 2,
        106,

        x + 2,
        106,

        HudDim
    );


    // Neutral.
    DrawLine(
        x - 6,
        center,

        x + 6,
        center,

        HudWhite
    );


    // -50 %
    DrawLine(
        x - 2,
        166,

        x + 2,
        166,

        HudDim
    );


    // -100 %
    DrawLine(
        x - 3,
        bottom,

        x + 3,
        bottom,

        HudDim
    );


    const short halfRange =
        center -
        top;


    const short markerY =
        static_cast<short>(
            center -
            throttle *
            static_cast<float>(
                halfRange
            )
        );


    DrawLine(
        x - 7,
        markerY,

        x + 7,
        markerY,

        HudOrange
    );


    DrawLine(
        x - 6,
        markerY - 1,

        x + 6,
        markerY - 1,

        HudOrange
    );
}


void HudRenderer::DrawTargetTriangle(
    const HudTargetMarker& target
) const
{
    if (
        !target.selected ||
        !target.visible
    )
    {
        return;
    }


    float directionX =
        target.directionX;

    float directionY =
        target.directionY;


    float directionLength =
        std::sqrt(
            directionX *
            directionX +
            directionY *
            directionY
        );


    /*
        Ha a target pont a kamera felé vagy
        attól elfelé néz, a forward vetülete
        gyakorlatilag ponttá esik össze.
    */
    if (
        !target.directionValid ||
        directionLength < 0.001f
    )
    {
        directionX =
            0.0f;

        directionY =
            -1.0f;

        directionLength =
            1.0f;
    }


    directionX /=
        directionLength;

    directionY /=
        directionLength;


    const float perpendicularX =
        -directionY;

    const float perpendicularY =
        directionX;


    float size =
        21.0f;


    if (target.depth < 45.0f)
    {
        size =
            29.0f;
    }
    else if (target.depth > 140.0f)
    {
        size =
            15.0f;
    }


    /*
        A háromszög csúcsa mindig
        a target projected forward
        irányába néz.
    */
    const float apexX =
        target.x +
        directionX *
        size;

    const float apexY =
        target.y +
        directionY *
        size;


    const float backCenterX =
        target.x -
        directionX *
        size *
        0.55f;

    const float backCenterY =
        target.y -
        directionY *
        size *
        0.55f;


    const float halfBase =
        size *
        0.72f;


    const float leftX =
        backCenterX +
        perpendicularX *
        halfBase;

    const float leftY =
        backCenterY +
        perpendicularY *
        halfBase;


    const float rightX =
        backCenterX -
        perpendicularX *
        halfBase;

    const float rightY =
        backCenterY -
        perpendicularY *
        halfBase;


    DrawLine(
        static_cast<short>(apexX),
        static_cast<short>(apexY),

        static_cast<short>(leftX),
        static_cast<short>(leftY),

        HudOrange
    );


    DrawLine(
        static_cast<short>(leftX),
        static_cast<short>(leftY),

        static_cast<short>(rightX),
        static_cast<short>(rightY),

        HudOrange
    );


    DrawLine(
        static_cast<short>(rightX),
        static_cast<short>(rightY),

        static_cast<short>(apexX),
        static_cast<short>(apexY),

        HudOrange
    );
}


void HudRenderer::DrawWeaponReticle(
    const HudWeaponReticle& reticle
) const
{
    if (!reticle.visible)
    {
        return;
    }


    const unsigned int color =
        reticle.impact
            ? HudOrange
            : HudWhite;


    const short x =
        static_cast<short>(
            reticle.x
        );

    const short y =
        static_cast<short>(
            reticle.y
        );


    constexpr short outer =
        7;

    constexpr short inner =
        2;


    DrawLine(
        x - outer,
        y,

        x - inner,
        y,

        color
    );


    DrawLine(
        x + inner,
        y,

        x + outer,
        y,

        color
    );


    DrawLine(
        x,
        y - outer,

        x,
        y - inner,

        color
    );


    DrawLine(
        x,
        y + inner,

        x,
        y + outer,

        color
    );
}


void HudRenderer::Draw(
    const HudFrameData& data
) const
{
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


    DrawThrottle(
        data.throttle
    );


    DrawTargetTriangle(
        data.target
    );


    for (
        int i = 0;
        i <
        data.weaponReticleCount;
        ++i
    )
    {
        DrawWeaponReticle(
            data.weaponReticles[i]
        );
    }


    sceGuEnable(
        GU_DEPTH_TEST
    );
}