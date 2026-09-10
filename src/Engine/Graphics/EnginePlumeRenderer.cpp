#include "EnginePlumeRenderer.h"

#include <pspgu.h>
#include <pspgum.h>

#include <cmath>


namespace
{
    constexpr float Pi =
        3.14159265358979323846f;

    constexpr int PlaneCount = 3;
    constexpr int VerticesPerPlane = 6;


    struct PlumeVertex
    {
        unsigned int color;

        float x;
        float y;
        float z;
    };


    ScePspFVector3 Add(
        const ScePspFVector3& a,
        const ScePspFVector3& b
    )
    {
        return
        {
            a.x + b.x,
            a.y + b.y,
            a.z + b.z
        };
    }


    ScePspFVector3 Multiply(
        const ScePspFVector3& value,
        float scalar
    )
    {
        return
        {
            value.x * scalar,
            value.y * scalar,
            value.z * scalar
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


    ScePspFVector3 Normalize(
        const ScePspFVector3& value
    )
    {
        const float length =
            std::sqrt(
                value.x * value.x +
                value.y * value.y +
                value.z * value.z
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
            1.0f / length;


        return
        {
            value.x * inverse,
            value.y * inverse,
            value.z * inverse
        };
    }


    void WriteVertex(
        PlumeVertex& destination,
        unsigned int color,
        const ScePspFVector3& position
    )
    {
        destination.color = color;

        destination.x = position.x;
        destination.y = position.y;
        destination.z = position.z;
    }


    void WritePlane(
        PlumeVertex* vertices,

        const ScePspFVector3& origin,
        const ScePspFVector3& tip,

        const ScePspFVector3& axis,

        float baseWidth,
        float tipWidth,

        unsigned int baseColor,
        unsigned int tipColor
    )
    {
        const ScePspFVector3 baseOffset =
            Multiply(
                axis,
                baseWidth
            );


        const ScePspFVector3 tipOffset =
            Multiply(
                axis,
                tipWidth
            );


        const ScePspFVector3 baseLeft =
            Add(
                origin,
                Multiply(
                    baseOffset,
                    -1.0f
                )
            );


        const ScePspFVector3 baseRight =
            Add(
                origin,
                baseOffset
            );


        const ScePspFVector3 tipLeft =
            Add(
                tip,
                Multiply(
                    tipOffset,
                    -1.0f
                )
            );


        const ScePspFVector3 tipRight =
            Add(
                tip,
                tipOffset
            );


        // Triangle 1
        WriteVertex(
            vertices[0],
            baseColor,
            baseLeft
        );

        WriteVertex(
            vertices[1],
            baseColor,
            baseRight
        );

        WriteVertex(
            vertices[2],
            tipColor,
            tipRight
        );


        // Triangle 2
        WriteVertex(
            vertices[3],
            baseColor,
            baseLeft
        );

        WriteVertex(
            vertices[4],
            tipColor,
            tipRight
        );

        WriteVertex(
            vertices[5],
            tipColor,
            tipLeft
        );
    }


    void DrawLayer(
        const ScePspFVector3& position,
        const ScePspFVector3& forward,

        float length,
        float baseWidth,
        float tipWidth,

        unsigned int baseColor,
        unsigned int tipColor
    )
    {
        const int vertexCount =
            PlaneCount *
            VerticesPerPlane;


        PlumeVertex* vertices =
            static_cast<PlumeVertex*>(
                sceGuGetMemory(
                    sizeof(PlumeVertex) *
                    vertexCount
                )
            );


        if (vertices == nullptr)
        {
            return;
        }


        const ScePspFVector3 normalizedForward =
            Normalize(
                forward
            );


        /*
            Keresünk két egymásra és a
            hajtómű irányára merőleges
            tengelyt.
        */
        ScePspFVector3 referenceUp =
        {
            0.0f,
            1.0f,
            0.0f
        };


        if (
            std::fabs(
                normalizedForward.y
            ) > 0.90f
        )
        {
            referenceUp =
            {
                1.0f,
                0.0f,
                0.0f
            };
        }


        const ScePspFVector3 axisA =
            Normalize(
                Cross(
                    normalizedForward,
                    referenceUp
                )
            );


        const ScePspFVector3 axisB =
            Normalize(
                Cross(
                    normalizedForward,
                    axisA
                )
            );


        const ScePspFVector3 tip =
            Add(
                position,
                Multiply(
                    normalizedForward,
                    length
                )
            );


        for (
            int plane = 0;
            plane < PlaneCount;
            ++plane
        )
        {
            const float angle =
                (
                    Pi /
                    static_cast<float>(
                        PlaneCount
                    )
                ) *
                static_cast<float>(
                    plane
                );


            const float cosine =
                std::cos(angle);

            const float sine =
                std::sin(angle);


            const ScePspFVector3 axis =
            {
                axisA.x * cosine +
                    axisB.x * sine,

                axisA.y * cosine +
                    axisB.y * sine,

                axisA.z * cosine +
                    axisB.z * sine
            };


            WritePlane(
                &vertices[
                    plane *
                    VerticesPerPlane
                ],

                position,
                tip,

                axis,

                baseWidth,
                tipWidth,

                baseColor,
                tipColor
            );
        }


        sceGumDrawArray(
            GU_TRIANGLES,

            GU_COLOR_8888 |
            GU_VERTEX_32BITF |
            GU_TRANSFORM_3D,

            vertexCount,

            nullptr,
            vertices
        );
    }
}


void EnginePlumeRenderer::Draw(
    const ScePspFVector3& position,
    const ScePspFVector3& forward,
    float power,
    float phase
) const
{
    if (power <= 0.001f)
    {
        return;
    }


    if (power > 1.0f)
    {
        power = 1.0f;
    }


    /*
        Kis, szabálytalan pulzálás.

        Később ezt válthatja fel az
        animált hajtómű-textúra.
    */
    const float flicker =
        1.0f +
        std::sin(
            phase
        ) * 0.055f +
        std::sin(
            phase * 2.73f
        ) * 0.025f;


    const float outerLength =
        (
            1.2f +
            power * 4.3f
        ) *
        flicker;


    const float coreLength =
        outerLength *
        0.68f;


    const float outerWidth =
        0.28f +
        power * 0.24f;


    const float coreWidth =
        outerWidth *
        0.48f;


    // --------------------------------------------------------
    // Render state
    // --------------------------------------------------------

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


    /*
        Additive blending:

        result =
            source * sourceAlpha +
            destination

        A PSP-n az 1.0 destination factorhoz
        GU_FIX + 0xFFFFFFFF használható.
    */
    sceGuBlendFunc(
        GU_ADD,
        GU_SRC_ALPHA,
        GU_FIX,
        0,
        0xFFFFFFFF
    );


    /*
        A hajó továbbra is kitakarhatja a
        mögötte lévő csóvát, tehát depth test
        marad.

        Viszont maga az áttetsző csóva ne
        írjon a depth bufferbe.
    */
    sceGuDepthMask(
        GU_TRUE
    );


    // --------------------------------------------------------
    // Outer blue glow
    // --------------------------------------------------------

    DrawLayer(
        position,
        forward,

        outerLength,

        outerWidth,
        0.06f,

        0xA0FFE0A0,
        0x08FF5000
    );


    // --------------------------------------------------------
    // Bright inner core
    // --------------------------------------------------------

    DrawLayer(
        position,
        forward,

        coreLength,

        coreWidth,
        0.025f,

        0xE8FFFFFF,
        0x18FFE8B0
    );


    // --------------------------------------------------------
    // Restore
    // --------------------------------------------------------

    sceGuDepthMask(
        GU_FALSE
    );


    sceGuDisable(
        GU_BLEND
    );
}