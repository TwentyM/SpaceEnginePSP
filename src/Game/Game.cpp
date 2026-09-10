#include "Game.h"

#include "../Engine/Input/Input.h"
#include "../Engine/Graphics/Renderer.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>

#include <cmath>


Game::Game()
    : m_shipRotation(0.0f),
      m_enginePower(0.25f),
      m_enginePhase(0.0f)
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


    // --------------------------------------------------------
    // TEMPORARY ENGINE POWER TEST
    // --------------------------------------------------------

    float targetPower =
        0.25f;


    // Triangle = normál előremenet.
    if (
        input.IsDown(
            PSP_CTRL_TRIANGLE
        )
    )
    {
        targetPower =
            0.70f;
    }


    // Circle = boost.
    if (
        input.IsDown(
            PSP_CTRL_CIRCLE
        )
    )
    {
        targetPower =
            1.0f;
    }


    /*
        Lágy átmenet, hogy ne azonnal
        ugorjon a csóva mérete.
    */
    float response =
        deltaTime *
        6.0f;


    if (response > 1.0f)
    {
        response = 1.0f;
    }


    m_enginePower +=
        (
            targetPower -
            m_enginePower
        ) *
        response;


    m_enginePhase +=
        deltaTime *
        (
            10.0f +
            m_enginePower *
            12.0f
        );
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


    // --------------------------------------------------------
    // MATERIAL
    // --------------------------------------------------------


    sceGuModelColor(
        0x00000000,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0x00000000
    );


    // --------------------------------------------------------
    // SIDEWINDER
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


    /*
        Csak preview scale!

        A modell tényleges méretét megtartjuk
        a .pspmesh-ben. Jelenleg a kamera még
        a régi kis tesztmodellhez van állítva.
    */
    ScePspFVector3 scale =
    {
        0.15f,
        0.15f,
        0.15f
    };


    sceGumScale(
        &scale
    );

    m_shipTexture.Bind();

    m_shipMesh.Draw();

    m_shipTexture.Unbind();

    // --------------------------------------------------------
    // ENGINE PLUMES
    // --------------------------------------------------------

    m_sun.Disable();


    int engineIndex = 0;


    for (
        int i = 0;
        i < m_shipResource.GetMarkerCount();
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


        /*
            A két hajtómű kap egy kis eltérő
            phase-t, így nem teljesen egyszerre
            pulzálnak.
        */
        const float phase =
            m_enginePhase +
            static_cast<float>(
                engineIndex
            ) *
            1.37f;


        m_enginePlume.Draw(
            marker->position,
            marker->forward,
            m_enginePower,
            phase
        );


        ++engineIndex;
    }
}