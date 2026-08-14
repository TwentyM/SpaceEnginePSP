#include "Game.h"

#include "../Engine/Input/Input.h"
#include "../Engine/Graphics/Renderer.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>

struct Vertex
{
    unsigned int color;

    float x;
    float y;
    float z;
};

static Vertex __attribute__((aligned(16))) cubeVertices[] =
{
    // Front
    {0xFF0000FF, -1, -1,  1},
    {0xFF0000FF,  1, -1,  1},
    {0xFF0000FF,  1,  1,  1},

    {0xFF0000FF, -1, -1,  1},
    {0xFF0000FF,  1,  1,  1},
    {0xFF0000FF, -1,  1,  1},


    // Back
    {0xFF00FF00,  1, -1, -1},
    {0xFF00FF00, -1, -1, -1},
    {0xFF00FF00, -1,  1, -1},

    {0xFF00FF00,  1, -1, -1},
    {0xFF00FF00, -1,  1, -1},
    {0xFF00FF00,  1,  1, -1},


    // Left
    {0xFFFF0000, -1, -1, -1},
    {0xFFFF0000, -1, -1,  1},
    {0xFFFF0000, -1,  1,  1},

    {0xFFFF0000, -1, -1, -1},
    {0xFFFF0000, -1,  1,  1},
    {0xFFFF0000, -1,  1, -1},


    // Right
    {0xFF00FFFF, 1, -1,  1},
    {0xFF00FFFF, 1, -1, -1},
    {0xFF00FFFF, 1,  1, -1},

    {0xFF00FFFF, 1, -1,  1},
    {0xFF00FFFF, 1,  1, -1},
    {0xFF00FFFF, 1,  1,  1},


    // Top
    {0xFFFF00FF, -1, 1,  1},
    {0xFFFF00FF,  1, 1,  1},
    {0xFFFF00FF,  1, 1, -1},

    {0xFFFF00FF, -1, 1,  1},
    {0xFFFF00FF,  1, 1, -1},
    {0xFFFF00FF, -1, 1, -1},


    // Bottom
    {0xFFFFFF00, -1, -1, -1},
    {0xFFFFFF00,  1, -1, -1},
    {0xFFFFFF00,  1, -1,  1},

    {0xFFFFFF00, -1, -1, -1},
    {0xFFFFFF00,  1, -1,  1},
    {0xFFFFFF00, -1, -1,  1}
};

Game::Game()
    : m_cubeRotation(0.0f)
{
}

void Game::Initialize()
{
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


    // Model matrix.
    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();


    ScePspFVector3 rotation =
    {
        m_cubeRotation * 0.5f,
        m_cubeRotation,
        0.0f
    };

    sceGumRotateXYZ(&rotation);


    sceGumDrawArray(
        GU_TRIANGLES,

        GU_COLOR_8888 |
        GU_VERTEX_32BITF |
        GU_TRANSFORM_3D,

        sizeof(cubeVertices) / sizeof(Vertex),

        nullptr,

        cubeVertices
    );
}