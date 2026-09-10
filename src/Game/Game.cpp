#include "Game.h"

#include "../Engine/Input/Input.h"
#include "../Engine/Graphics/Renderer.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>


Game::Game()
    : m_enginePhase(0.0f),
    m_fireRequested(false)
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
    // SIDEWINDER
    // --------------------------------------------------------

    if (
        m_shipResource.Load(
            "assets/ships/Sidewinder.pspmesh"
        )
    )
    {
        m_shipResource.BindTo(
            m_shipMesh
        );
    }


    m_shipTexture.Load(
        "assets/ships/Sidewinder.psptx"
    );


    // --------------------------------------------------------
    // CAMERA INITIAL POSITION
    // --------------------------------------------------------

    m_camera.Follow(
        m_playerShip.GetPosition(),
        m_playerShip.GetOrientation()
    );


    sceKernelDcacheWritebackAll();
}


void Game::Update(
    const Input& input,
    float deltaTime
)
{
    // --------------------------------------------------------
    // PLAYER SHIP
    // --------------------------------------------------------

    m_playerShip.Update(
        input,
        deltaTime
    );


    // --------------------------------------------------------
    // CAMERA
    // --------------------------------------------------------

    m_camera.Follow(
        m_playerShip.GetPosition(),
        m_playerShip.GetOrientation()
    );


    // --------------------------------------------------------
    // ENGINE EFFECT
    // --------------------------------------------------------

    const float enginePower =
        m_playerShip.GetEnginePower();


    m_enginePhase +=
        deltaTime *
        (
            10.0f +
            enginePower *
            12.0f
        );


    // --------------------------------------------------------
    // PROJECTILES
    // --------------------------------------------------------

    m_projectiles.Update(
        deltaTime
    );

    if (
        input.IsPressed(
            PSP_CTRL_START
        )
    )
    {
        m_fireRequested =
            true;
    }
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
    // STARFIELD
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


    sceGuModelColor(
        0x00000000,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0x00000000
    );


    // --------------------------------------------------------
    // PLAYER SHIP TRANSFORM
    // --------------------------------------------------------

    m_playerShip.ApplyModelTransform();


    /*
        Most már nincs 0.15 preview scale.

        A Blender modell tényleges méretét
        használjuk a világban.
    */


    ScePspFMatrix4 shipModelMatrix;


    sceGumStoreMatrix(
        &shipModelMatrix
    );


    // --------------------------------------------------------
    // SIDEWINDER
    // --------------------------------------------------------

    m_shipTexture.Bind();


    m_shipMesh.Draw();


    m_shipTexture.Unbind();


    // --------------------------------------------------------
    // WEAPON FIRE
    // --------------------------------------------------------

    if (m_fireRequested)
    {
        for (
            int i = 0;
            i <
            m_shipResource.GetMarkerCount();
            ++i
        )
        {
            const PspMeshMarker* marker =
                m_shipResource.GetMarker(
                    i
                );


            if (
                marker == nullptr ||
                marker->type !=
                    PspMeshMarkerType::Weapon
            )
            {
                continue;
            }


            m_projectiles.Spawn(
                shipModelMatrix,
                marker->position,
                marker->forward
            );
        }


        m_fireRequested =
            false;
    }


    // --------------------------------------------------------
    // ENGINE PLUMES
    // --------------------------------------------------------

    m_sun.Disable();


    int engineIndex =
        0;


    const float enginePower =
        m_playerShip.GetEnginePower();


    for (
        int i = 0;
        i <
        m_shipResource.GetMarkerCount();
        ++i
    )
    {
        const PspMeshMarker* marker =
            m_shipResource.GetMarker(
                i
            );


        if (
            marker == nullptr ||
            marker->type !=
                PspMeshMarkerType::Engine
        )
        {
            continue;
        }


        const float phase =
            m_enginePhase +
            static_cast<float>(
                engineIndex
            ) *
            1.37f;


        m_enginePlume.Draw(
            marker->position,
            marker->forward,
            enginePower,
            phase
        );


        ++engineIndex;
    }


    // --------------------------------------------------------
    // PROJECTILES
    // --------------------------------------------------------

    m_projectiles.Draw();
}