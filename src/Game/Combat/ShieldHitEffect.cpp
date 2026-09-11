#include "ShieldHitEffect.h"

#include <pspgu.h>
#include <pspgum.h>

#include <cmath>


namespace
{
    constexpr float Pi =
        3.14159265358979323846f;

    constexpr int RingSegments =
        40;


    struct ShieldVertex
    {
        unsigned int color;

        float x;
        float y;
        float z;
    };


    float Dot(
        const ScePspFVector3& a,
        const ScePspFVector3& b
    )
    {
        return
            a.x * b.x +
            a.y * b.y +
            a.z * b.z;
    }


    ScePspFVector3 Normalize(
        const ScePspFVector3& value
    )
    {
        const float length =
            std::sqrt(
                Dot(
                    value,
                    value
                )
            );


        if (length < 0.00001f)
        {
            return
            {
                0.0f,
                0.0f,
                1.0f
            };
        }


        const float inverse =
            1.0f /
            length;


        return
        {
            value.x * inverse,
            value.y * inverse,
            value.z * inverse
        };
    }


    ScePspFVector3 Cross(
        const ScePspFVector3& a,
        const ScePspFVector3& b
    )
    {
        return
        {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }


    void DrawRing(
        const ScePspFVector3& center,

        float radius,

        const ScePspFVector3& normal,

        float angle,

        unsigned int color
    )
    {
        ScePspFVector3 reference =
        {
            0.0f,
            1.0f,
            0.0f
        };


        if (
            std::fabs(
                normal.y
            ) > 0.9f
        )
        {
            reference =
            {
                1.0f,
                0.0f,
                0.0f
            };
        }


        const ScePspFVector3 axisA =
            Normalize(
                Cross(
                    normal,
                    reference
                )
            );


        const ScePspFVector3 axisB =
            Normalize(
                Cross(
                    normal,
                    axisA
                )
            );


        constexpr int VertexCount =
            RingSegments + 1;


        ShieldVertex* vertices =
            static_cast<ShieldVertex*>(
                sceGuGetMemory(
                    sizeof(ShieldVertex) *
                    VertexCount
                )
            );


        if (vertices == nullptr)
        {
            return;
        }


        const float ringCenterScale =
            std::cos(
                angle
            );


        const float ringRadiusScale =
            std::sin(
                angle
            );


        for (
            int i = 0;
            i <= RingSegments;
            ++i
        )
        {
            const float around =
                (
                    2.0f *
                    Pi *
                    static_cast<float>(i)
                ) /
                static_cast<float>(
                    RingSegments
                );


            const float c =
                std::cos(
                    around
                );


            const float s =
                std::sin(
                    around
                );


            const ScePspFVector3 tangent =
            {
                axisA.x * c +
                    axisB.x * s,

                axisA.y * c +
                    axisB.y * s,

                axisA.z * c +
                    axisB.z * s
            };


            ShieldVertex& vertex =
                vertices[i];


            vertex.color =
                color;


            vertex.x =
                center.x +
                radius *
                (
                    normal.x *
                    ringCenterScale +
                    tangent.x *
                    ringRadiusScale
                );


            vertex.y =
                center.y +
                radius *
                (
                    normal.y *
                    ringCenterScale +
                    tangent.y *
                    ringRadiusScale
                );


            vertex.z =
                center.z +
                radius *
                (
                    normal.z *
                    ringCenterScale +
                    tangent.z *
                    ringRadiusScale
                );
        }


        sceGumDrawArray(
            GU_LINE_STRIP,

            GU_COLOR_8888 |
            GU_VERTEX_32BITF |
            GU_TRANSFORM_3D,

            VertexCount,

            nullptr,
            vertices
        );
    }
}


ShieldHitEffect::ShieldHitEffect()
{
    for (
        int i = 0;
        i < MaxHits;
        ++i
    )
    {
        m_hits[i].active =
            false;
    }
}


void ShieldHitEffect::Trigger(
    const ScePspFVector3& normal
)
{
    int selected =
        -1;


    for (
        int i = 0;
        i < MaxHits;
        ++i
    )
    {
        if (!m_hits[i].active)
        {
            selected =
                i;

            break;
        }
    }


    /*
        Ha minden slot foglalt,
        a legrégebbit használjuk újra.
    */
    if (selected < 0)
    {
        float oldest =
            -1.0f;


        for (
            int i = 0;
            i < MaxHits;
            ++i
        )
        {
            if (
                m_hits[i].age >
                oldest
            )
            {
                oldest =
                    m_hits[i].age;

                selected =
                    i;
            }
        }
    }


    Hit& hit =
        m_hits[selected];


    hit.active =
        true;

    hit.normal =
        Normalize(
            normal
        );

    hit.age =
        0.0f;

    hit.duration =
        0.75f;
}


void ShieldHitEffect::Update(
    float deltaTime
)
{
    for (
        int i = 0;
        i < MaxHits;
        ++i
    )
    {
        Hit& hit =
            m_hits[i];


        if (!hit.active)
        {
            continue;
        }


        hit.age +=
            deltaTime;


        if (
            hit.age >=
            hit.duration
        )
        {
            hit.active =
                false;
        }
    }
}


void ShieldHitEffect::Draw(
    const ScePspFVector3& center,
    float radius
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


    sceGuEnable(
        GU_BLEND
    );


    sceGuBlendFunc(
        GU_ADD,
        GU_SRC_ALPHA,
        GU_FIX,
        0,
        0xFFFFFFFF
    );


    sceGuDepthMask(
        GU_TRUE
    );


    sceGumMatrixMode(
        GU_MODEL
    );

    sceGumLoadIdentity();


    /*
        Picit nagyobb, mint a matematikai
        collision sphere, hogy ne érjen
        bele a hajó geometriájába.
    */
    const float drawRadius =
        radius *
        1.035f;


    for (
        int i = 0;
        i < MaxHits;
        ++i
    )
    {
        const Hit& hit =
            m_hits[i];


        if (!hit.active)
        {
            continue;
        }


        float progress =
            hit.age /
            hit.duration;


        if (progress > 1.0f)
        {
            progress =
                1.0f;
        }


        /*
            0° = becsapódási pont

            A hullám majdnem a gömb
            túloldaláig végigfut.
        */
        const float angle =
            0.04f +
            progress *
            (
                Pi *
                0.92f
            );


        const unsigned int alpha =
            static_cast<unsigned int>(
                220.0f *
                (
                    1.0f -
                    progress
                )
            );


        const unsigned int color =
            (
                alpha << 24
            ) |
            0x00FFD080;


        DrawRing(
            center,
            drawRadius,
            hit.normal,
            angle,
            color
        );


        /*
            Egy halvány második hullám
            vastagabb/glow-szerű hatáshoz.
        */
        if (angle > 0.06f)
        {
            const unsigned int softAlpha =
                alpha / 3;


            const unsigned int softColor =
                (
                    softAlpha << 24
                ) |
                0x00FFD080;


            DrawRing(
                center,
                drawRadius,
                hit.normal,

                angle - 0.035f,

                softColor
            );
        }
    }


    sceGuDepthMask(
        GU_FALSE
    );


    sceGuDisable(
        GU_BLEND
    );
}