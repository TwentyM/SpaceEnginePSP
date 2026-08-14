#include "Starfield.h"

#include <pspgu.h>
#include <pspgum.h>

#include <cmath>

namespace
{
    std::uint32_t NextRandom(
        std::uint32_t& state
    )
    {
        state =
            state * 1664525u +
            1013904223u;

        return state;
    }

    float RandomSigned(
        std::uint32_t& state
    )
    {
        const std::uint32_t value =
            (NextRandom(state) >> 8) &
            0xFFFFu;

        return
            static_cast<float>(value) /
            32767.5f -
            1.0f;
    }
}

Starfield::Starfield()
{
}

void Starfield::Generate(
    std::uint32_t seed
)
{
    std::uint32_t randomState = seed;

    for (int i = 0; i < StarCount; ++i)
    {
        float x;
        float y;
        float z;

        float lengthSquared;

        do
        {
            x = RandomSigned(randomState);
            y = RandomSigned(randomState);
            z = RandomSigned(randomState);

            lengthSquared =
                x * x +
                y * y +
                z * z;
        }
        while (
            lengthSquared < 0.05f ||
            lengthSquared > 1.0f
        );

        const float inverseLength =
            1.0f /
            std::sqrt(lengthSquared);

        const float radius =
            80.0f +
            static_cast<float>(
                NextRandom(randomState) %
                50u
            );

        m_vertices[i].x =
            x *
            inverseLength *
            radius;

        m_vertices[i].y =
            y *
            inverseLength *
            radius;

        m_vertices[i].z =
            z *
            inverseLength *
            radius;

        switch (
            NextRandom(randomState) %
            16u
        )
        {
            case 0:
                // Enyhén kék.
                m_vertices[i].color =
                    0xFFFFD8C8;
                break;

            case 1:
                // Enyhén meleg.
                m_vertices[i].color =
                    0xFFB8E4FF;
                break;

            case 2:
            case 3:
                m_vertices[i].color =
                    0xFFFFFFFF;
                break;

            default:
                m_vertices[i].color =
                    0xFFD8D8D8;
                break;
        }
    }
}

void Starfield::Draw(
    const ScePspFVector3& cameraPosition
) const
{
    sceGuDisable(GU_LIGHTING);
    sceGuDisable(GU_DEPTH_TEST);

    sceGumMatrixMode(
        GU_MODEL
    );

    sceGumLoadIdentity();

    /*
        A csillagmezőt együtt mozgatjuk
        a kamerával.

        Így a távoli csillagoknak nincs
        érzékelhető parallaxisa és nem tudunk
        "kirepülni" a csillaggömbből.
    */
    sceGumTranslate(
        &cameraPosition
    );

    sceGumDrawArray(
        GU_POINTS,

        GU_COLOR_8888 |
        GU_VERTEX_32BITF |
        GU_TRANSFORM_3D,

        StarCount,

        nullptr,

        m_vertices
    );

    sceGuEnable(
        GU_DEPTH_TEST
    );
}