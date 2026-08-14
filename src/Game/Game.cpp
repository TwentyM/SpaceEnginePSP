#include "Game.h"

#include "../Engine/Input/Input.h"
#include "../Engine/Graphics/Renderer.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>

#include <cmath>
#include <cstdint>

namespace
{
    constexpr int StarCount = 192;

    struct LitVertex
    {
        // PSP GU vertex member order:
        // normal -> position when no texture/color data is present.
        float nx;
        float ny;
        float nz;

        float x;
        float y;
        float z;
    };

    struct StarVertex
    {
        unsigned int color;

        float x;
        float y;
        float z;
    };

    // One normal per face. This is intentionally a hard-edged test mesh:
    // later Blender models will provide their own vertex normals.
    LitVertex __attribute__((aligned(16))) cubeVertices[] =
    {
        // Front (+Z)
        { 0,  0,  1,   -1, -1,  1},
        { 0,  0,  1,    1, -1,  1},
        { 0,  0,  1,    1,  1,  1},
        { 0,  0,  1,   -1, -1,  1},
        { 0,  0,  1,    1,  1,  1},
        { 0,  0,  1,   -1,  1,  1},

        // Back (-Z)
        { 0,  0, -1,    1, -1, -1},
        { 0,  0, -1,   -1, -1, -1},
        { 0,  0, -1,   -1,  1, -1},
        { 0,  0, -1,    1, -1, -1},
        { 0,  0, -1,   -1,  1, -1},
        { 0,  0, -1,    1,  1, -1},

        // Left (-X)
        {-1,  0,  0,   -1, -1, -1},
        {-1,  0,  0,   -1, -1,  1},
        {-1,  0,  0,   -1,  1,  1},
        {-1,  0,  0,   -1, -1, -1},
        {-1,  0,  0,   -1,  1,  1},
        {-1,  0,  0,   -1,  1, -1},

        // Right (+X)
        { 1,  0,  0,    1, -1,  1},
        { 1,  0,  0,    1, -1, -1},
        { 1,  0,  0,    1,  1, -1},
        { 1,  0,  0,    1, -1,  1},
        { 1,  0,  0,    1,  1, -1},
        { 1,  0,  0,    1,  1,  1},

        // Top (+Y)
        { 0,  1,  0,   -1,  1,  1},
        { 0,  1,  0,    1,  1,  1},
        { 0,  1,  0,    1,  1, -1},
        { 0,  1,  0,   -1,  1,  1},
        { 0,  1,  0,    1,  1, -1},
        { 0,  1,  0,   -1,  1, -1},

        // Bottom (-Y)
        { 0, -1,  0,   -1, -1, -1},
        { 0, -1,  0,    1, -1, -1},
        { 0, -1,  0,    1, -1,  1},
        { 0, -1,  0,   -1, -1, -1},
        { 0, -1,  0,    1, -1,  1},
        { 0, -1,  0,   -1, -1,  1}
    };

    StarVertex __attribute__((aligned(16))) starVertices[StarCount];

    std::uint32_t NextRandom(std::uint32_t& state)
    {
        state = state * 1664525u + 1013904223u;
        return state;
    }

    float RandomSigned(std::uint32_t& state)
    {
        const std::uint32_t value = (NextRandom(state) >> 8) & 0xFFFFu;
        return static_cast<float>(value) / 32767.5f - 1.0f;
    }

    void GenerateStarfield()
    {
        std::uint32_t randomState = 0x5EED1234u;

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

                lengthSquared = x * x + y * y + z * z;
            }
            while (lengthSquared < 0.05f || lengthSquared > 1.0f);

            const float inverseLength = 1.0f / std::sqrt(lengthSquared);
            const float radius = 70.0f + static_cast<float>(NextRandom(randomState) % 50u);

            starVertices[i].x = x * inverseLength * radius;
            starVertices[i].y = y * inverseLength * radius;
            starVertices[i].z = z * inverseLength * radius;

            // PSP colors are ABGR. Most stars are neutral white/grey,
            // with a few subtle warm/cool variants.
            switch (NextRandom(randomState) % 12u)
            {
                case 0:
                    starVertices[i].color = 0xFFFFD8C8; // cool white
                    break;

                case 1:
                    starVertices[i].color = 0xFFB8E4FF; // warm white
                    break;

                case 2:
                case 3:
                    starVertices[i].color = 0xFFFFFFFF; // bright white
                    break;

                default:
                    starVertices[i].color = 0xFFD8D8D8; // soft white
                    break;
            }
        }
    }
}

Game::Game()
    : m_cubeRotation(0.0f)
{
}

void Game::Initialize()
{
    GenerateStarfield();

    // Vertex data is read by the Geometry Engine, so make sure generated
    // star data has reached memory before the first draw.
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

    m_cubeRotation +=
        0.35f *
        deltaTime;
}

void Game::Render(
    Renderer& renderer
)
{
    (void)renderer;

    m_camera.Apply();

    // --------------------------------------------------------
    // STARFIELD
    // --------------------------------------------------------
    // GU_POINTS are single-pixel primitives, which makes them a very cheap
    // first approximation of distant stars. Lighting/depth are disabled so
    // the starfield behaves like a background layer.

    sceGuDisable(GU_LIGHTING);
    sceGuDisable(GU_DEPTH_TEST);

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    sceGumDrawArray(
        GU_POINTS,

        GU_COLOR_8888 |
        GU_VERTEX_32BITF |
        GU_TRANSFORM_3D,

        StarCount,
        nullptr,
        starVertices
    );

    sceGuEnable(GU_DEPTH_TEST);

    // --------------------------------------------------------
    // SIMPLE SUN / DIRECTIONAL LIGHT
    // --------------------------------------------------------

    sceGuEnable(GU_LIGHTING);
    sceGuEnable(GU_LIGHT0);

    sceGuLightMode(GU_SINGLE_COLOR);

    // Global low-level ambient illumination so the unlit side never becomes
    // completely black. Later this can depend on system/environment type.
    sceGuAmbient(0xFF202020);

    const ScePspFVector3 sunDirection =
    {
        -0.55f,
        -0.35f,
        -0.75f
    };

    sceGuLight(
        0,
        GU_DIRECTIONAL,
        GU_AMBIENT_AND_DIFFUSE,
        &sunDirection
    );

    sceGuLightColor(
        0,
        GU_AMBIENT,
        0xFF181818
    );

    sceGuLightColor(
        0,
        GU_DIFFUSE,
        0xFFFFFFFF
    );

    // Grey test material. No vertex colors are needed here: the lighting is
    // driven by the normals, which is exactly what we need to validate before
    // importing real ship meshes.
    sceGuModelColor(
        0x00000000,
        0xFFFFFFFF,
        0xFFD0D0D0,
        0x00000000
    );

    // --------------------------------------------------------
    // LIT TEST CUBE
    // --------------------------------------------------------

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    const ScePspFVector3 rotation =
    {
        m_cubeRotation * 0.5f,
        m_cubeRotation,
        0.0f
    };

    sceGumRotateXYZ(&rotation);

    sceGumDrawArray(
        GU_TRIANGLES,

        GU_NORMAL_32BITF |
        GU_VERTEX_32BITF |
        GU_TRANSFORM_3D,

        sizeof(cubeVertices) / sizeof(LitVertex),
        nullptr,
        cubeVertices
    );

    // Restore state for future render passes.
    sceGuDisable(GU_LIGHT0);
    sceGuDisable(GU_LIGHTING);
}