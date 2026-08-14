#include "Game.h"

#include "../Engine/Input/Input.h"
#include "../Engine/Graphics/Renderer.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>

#include <cmath>

namespace
{
    struct Position
    {
        float x;
        float y;
        float z;
    };

    struct LitVertex
    {
        float nx;
        float ny;
        float nz;

        float x;
        float y;
        float z;
    };

    struct Triangle
    {
        Position a;
        Position b;
        Position c;
    };


    /*
        Egy nagyon egyszerű, faceted low-poly teszthajó.

                    NOSE
                     /\
                    /  \
             ______/____\______
            /                  \
           /                    \
        __/                      \__
       /                            \
       \_______              _______/
               \____________/


        Nem szép modellnek készül,
        hanem világítás és mesh teszthez.
    */

    constexpr Triangle ShipTriangles[] =
    {
        // ----------------------------------------------------
        // TOP FRONT
        // ----------------------------------------------------

        {
            { 0.00f,  0.00f, -2.20f},
            { 0.70f, -0.20f, -0.70f},
            { 0.00f,  0.55f, -0.55f}
        },

        {
            { 0.00f,  0.00f, -2.20f},
            { 0.00f,  0.55f, -0.55f},
            {-0.70f, -0.20f, -0.70f}
        },


        // ----------------------------------------------------
        // LEFT TOP
        // ----------------------------------------------------

        {
            {-0.70f, -0.20f, -0.70f},
            { 0.00f,  0.55f, -0.55f},
            {-1.65f, -0.20f,  0.25f}
        },

        {
            {-1.65f, -0.20f,  0.25f},
            { 0.00f,  0.55f, -0.55f},
            { 0.00f,  0.45f,  0.95f}
        },


        // ----------------------------------------------------
        // RIGHT TOP
        // ----------------------------------------------------

        {
            { 0.00f,  0.55f, -0.55f},
            { 0.70f, -0.20f, -0.70f},
            { 1.65f, -0.20f,  0.25f}
        },

        {
            { 0.00f,  0.55f, -0.55f},
            { 1.65f, -0.20f,  0.25f},
            { 0.00f,  0.45f,  0.95f}
        },


        // ----------------------------------------------------
        // LEFT REAR TOP
        // ----------------------------------------------------

        {
            {-1.65f, -0.20f,  0.25f},
            { 0.00f,  0.45f,  0.95f},
            {-0.65f, -0.10f,  1.25f}
        },


        // ----------------------------------------------------
        // REAR TOP
        // ----------------------------------------------------

        {
            {-0.65f, -0.10f,  1.25f},
            { 0.00f,  0.45f,  0.95f},
            { 0.65f, -0.10f,  1.25f}
        },


        // ----------------------------------------------------
        // RIGHT REAR TOP
        // ----------------------------------------------------

        {
            { 0.00f,  0.45f,  0.95f},
            { 1.65f, -0.20f,  0.25f},
            { 0.65f, -0.10f,  1.25f}
        },


        // ----------------------------------------------------
        // BOTTOM FRONT
        // ----------------------------------------------------

        {
            { 0.00f,  0.00f, -2.20f},
            {-0.70f, -0.20f, -0.70f},
            { 0.00f, -0.45f,  0.85f}
        },

        {
            { 0.00f,  0.00f, -2.20f},
            { 0.00f, -0.45f,  0.85f},
            { 0.70f, -0.20f, -0.70f}
        },


        // ----------------------------------------------------
        // LEFT BOTTOM
        // ----------------------------------------------------

        {
            {-0.70f, -0.20f, -0.70f},
            {-1.65f, -0.20f,  0.25f},
            { 0.00f, -0.45f,  0.85f}
        },

        {
            {-1.65f, -0.20f,  0.25f},
            {-0.65f, -0.10f,  1.25f},
            { 0.00f, -0.45f,  0.85f}
        },


        // ----------------------------------------------------
        // RIGHT BOTTOM
        // ----------------------------------------------------

        {
            { 0.70f, -0.20f, -0.70f},
            { 0.00f, -0.45f,  0.85f},
            { 1.65f, -0.20f,  0.25f}
        },

        {
            { 1.65f, -0.20f,  0.25f},
            { 0.00f, -0.45f,  0.85f},
            { 0.65f, -0.10f,  1.25f}
        },


        // ----------------------------------------------------
        // REAR BOTTOM
        // ----------------------------------------------------

        {
            {-0.65f, -0.10f,  1.25f},
            { 0.65f, -0.10f,  1.25f},
            { 0.00f, -0.45f,  0.85f}
        }
    };


    constexpr int ShipTriangleCount =
        sizeof(ShipTriangles) /
        sizeof(Triangle);

    constexpr int ShipVertexCount =
        ShipTriangleCount * 3;


    alignas(16)
    LitVertex ShipVertices[
        ShipVertexCount
    ];


    void WriteVertex(
        LitVertex& destination,
        const Position& position,
        float nx,
        float ny,
        float nz
    )
    {
        destination.nx = nx;
        destination.ny = ny;
        destination.nz = nz;

        destination.x = position.x;
        destination.y = position.y;
        destination.z = position.z;
    }


    void BuildShipMesh()
    {
        for (
            int triangleIndex = 0;
            triangleIndex < ShipTriangleCount;
            ++triangleIndex
        )
        {
            const Triangle& triangle =
                ShipTriangles[
                    triangleIndex
                ];


            // Edge AB.
            const float ux =
                triangle.b.x -
                triangle.a.x;

            const float uy =
                triangle.b.y -
                triangle.a.y;

            const float uz =
                triangle.b.z -
                triangle.a.z;


            // Edge AC.
            const float vx =
                triangle.c.x -
                triangle.a.x;

            const float vy =
                triangle.c.y -
                triangle.a.y;

            const float vz =
                triangle.c.z -
                triangle.a.z;


            // Cross product.
            float nx =
                uy * vz -
                uz * vy;

            float ny =
                uz * vx -
                ux * vz;

            float nz =
                ux * vy -
                uy * vx;


            float length =
                std::sqrt(
                    nx * nx +
                    ny * ny +
                    nz * nz
                );


            if (length > 0.00001f)
            {
                nx /= length;
                ny /= length;
                nz /= length;
            }


            /*
                Ellenőrizzük, hogy a normal kifelé néz-e.

                A prototípushajó nagyjából az origó
                körül helyezkedik el, ezért a triangle
                centroidjával egyszerűen eldönthető,
                hogy befelé vagy kifelé mutat.
            */

            const float centerX =
                (
                    triangle.a.x +
                    triangle.b.x +
                    triangle.c.x
                ) / 3.0f;

            const float centerY =
                (
                    triangle.a.y +
                    triangle.b.y +
                    triangle.c.y
                ) / 3.0f;

            const float centerZ =
                (
                    triangle.a.z +
                    triangle.b.z +
                    triangle.c.z
                ) / 3.0f;


            const float direction =
                nx * centerX +
                ny * centerY +
                nz * centerZ;


            if (direction < 0.0f)
            {
                nx = -nx;
                ny = -ny;
                nz = -nz;
            }


            const int output =
                triangleIndex * 3;


            WriteVertex(
                ShipVertices[output + 0],
                triangle.a,
                nx,
                ny,
                nz
            );

            WriteVertex(
                ShipVertices[output + 1],
                triangle.b,
                nx,
                ny,
                nz
            );

            WriteVertex(
                ShipVertices[output + 2],
                triangle.c,
                nx,
                ny,
                nz
            );
        }
    }
}


Game::Game()
    : m_shipRotation(0.0f)
{
}


void Game::Initialize()
{
    // --------------------------------------------------------
    // STARFIELD
    // --------------------------------------------------------

    m_starfield.Generate(
        0x5EED1234u
    );


    // --------------------------------------------------------
    // SUN
    // --------------------------------------------------------

    m_sun.SetDirection(
        -0.55f,
        -0.35f,
        -0.75f
    );

    m_sun.SetGlobalAmbient(
        0xFF202020
    );

    m_sun.SetAmbient(
        0xFF181818
    );

    m_sun.SetDiffuse(
        0xFFFFFFFF
    );


    // --------------------------------------------------------
    // TEST SHIP
    // --------------------------------------------------------

    BuildShipMesh();

    m_shipMesh.SetData(
        ShipVertices,
        ShipVertexCount,

        GU_TRIANGLES,

        GU_NORMAL_32BITF |
        GU_VERTEX_32BITF |
        GU_TRANSFORM_3D
    );


    /*
        Starfield és ship vertexek CPU oldalon
        készültek, ezért flusholjuk a cache-t,
        mielőtt a Geometry Engine olvassa őket.
    */
    sceKernelDcacheWritebackAll();
}


void Game::Update(
    const Input& input,
    float deltaTime
)
{
    m_camera.Update(
        input,
        deltaTime
    );


    m_shipRotation +=
        0.30f *
        deltaTime;
}


void Game::Render(
    Renderer& renderer
)
{
    (void)renderer;


    // --------------------------------------------------------
    // CAMERA
    // --------------------------------------------------------

    m_camera.Apply();


    // --------------------------------------------------------
    // BACKGROUND STARS
    // --------------------------------------------------------

    const ScePspFVector3 cameraPosition =
        m_camera.GetPosition();

    m_starfield.Draw(
        cameraPosition
    );


    // --------------------------------------------------------
    // SUN
    // --------------------------------------------------------

    m_sun.Apply();


    // --------------------------------------------------------
    // MATERIAL
    // --------------------------------------------------------

    sceGuModelColor(
        0x00000000,
        0xFFFFFFFF,
        0xFFD0D0D0,
        0x00000000
    );


    // --------------------------------------------------------
    // SHIP
    // --------------------------------------------------------

    sceGumMatrixMode(
        GU_MODEL
    );

    sceGumLoadIdentity();


    ScePspFVector3 rotation =
    {
        m_shipRotation * 0.25f,
        m_shipRotation,
        m_shipRotation * 0.10f
    };

    sceGumRotateXYZ(
        &rotation
    );


    m_shipMesh.Draw();


    // --------------------------------------------------------
    // CLEANUP
    // --------------------------------------------------------

    m_sun.Disable();
}