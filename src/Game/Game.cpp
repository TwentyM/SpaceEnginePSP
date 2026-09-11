#include "Game.h"

#include "../Engine/Input/Input.h"
#include "../Engine/Graphics/Renderer.h"
#include "../Engine/Physics/SphereCollision.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>


Game::Game()
    : m_enginePhase(0.0f),
      m_playerTouchingTarget(false),
      m_targetSelected(false)
{
    m_targetPosition =
    {
        0.0f,
        0.0f,
        -90.0f
    };
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
    // PLAYER <-> TARGET COLLISION
    // --------------------------------------------------------

    const float collisionRadius =
        m_shipResource.GetBoundingRadius();


    bool touchingTarget =
        false;


    if (collisionRadius > 0.0f)
    {
        SphereCollisionResult collision;


        const ScePspFVector3 playerPosition =
            m_playerShip.GetPosition();


        if (
            SphereCollision::Test(
                playerPosition,
                collisionRadius,

                m_targetPosition,
                collisionRadius,

                collision
            )
        )
        {
            touchingTarget =
                true;


            /*
                A player került penetrációba
                a statikus targettel.

                Pontosan annyival toljuk ki,
                amennyi az átfedés.
            */
            ScePspFVector3 correction =
            {
                collision.normal.x *
                    collision.penetration,

                collision.normal.y *
                    collision.penetration,

                collision.normal.z *
                    collision.penetration
            };


            m_playerShip.AddWorldOffset(
                correction
            );


            /*
                Csak az első érintkezési
                frame-ben triggereljük.

                Ha nyomod tovább a gázt,
                nem indít minden frame-ben
                új ripple-t.
            */
            if (!m_playerTouchingTarget)
            {
                m_targetShield.Trigger(
                    collision.normal
                );
            }
        }
    }


    m_playerTouchingTarget =
        touchingTarget;


    // --------------------------------------------------------
    // CAMERA
    // --------------------------------------------------------

    m_camera.Follow(
        m_playerShip.GetPosition(),
        m_playerShip.GetOrientation()
    );


    // --------------------------------------------------------
    // TARGET SELECTION
    // --------------------------------------------------------

    if (
        input.IsPressed(
            PSP_CTRL_SELECT
        )
    )
    {
        /*
            Ha már ki van jelölve,
            SELECT = deselect.
        */
        if (m_targetSelected)
        {
            m_targetSelected =
                false;
        }
        else
        {
            float screenX =
                0.0f;

            float screenY =
                0.0f;

            float depth =
                0.0f;


            const bool visible =
                m_camera.ProjectWorldToScreen(
                    m_targetPosition,

                    screenX,
                    screenY,
                    depth
                );


            if (visible)
            {
                const float dx =
                    screenX -
                    240.0f;


                const float dy =
                    screenY -
                    136.0f;


                /*
                    SELECT csak akkor fogja meg
                    a targetet, ha nagyjából
                    a célkereszt közelében van.
                */
                constexpr float selectionRadius =
                    85.0f;


                if (
                    dx * dx +
                    dy * dy <=
                    selectionRadius *
                    selectionRadius
                )
                {
                    m_targetSelected =
                        true;
                }
            }
        }
    }


    // --------------------------------------------------------
    // ENGINE
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
    // FIRE
    // --------------------------------------------------------

    if (
        input.IsPressed(
            PSP_CTRL_START
        )
    )
    {
        const ScePspFVector3 shipPosition =
            m_playerShip.GetPosition();


        const ScePspFQuaternion shipOrientation =
            m_playerShip.GetOrientation();


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


            ScePspFVector3 rotatedPosition;


            gumRotateVector(
                &rotatedPosition,
                &shipOrientation,
                &marker->position
            );


            ScePspFVector3 worldPosition =
            {
                shipPosition.x +
                    rotatedPosition.x,

                shipPosition.y +
                    rotatedPosition.y,

                shipPosition.z +
                    rotatedPosition.z
            };


            ScePspFVector3 worldDirection;


            gumRotateVector(
                &worldDirection,
                &shipOrientation,
                &marker->forward
            );


            m_projectiles.Spawn(
                worldPosition,
                worldDirection
            );
        }
    }


    // --------------------------------------------------------
    // PROJECTILES
    // --------------------------------------------------------

    m_projectiles.Update(
        deltaTime
    );


    // --------------------------------------------------------
    // TARGET COLLISION
    // --------------------------------------------------------

    ProjectileHit hits[4];


    const int hitCount =
        m_projectiles.CheckSphereCollisions(
            m_targetPosition,

            m_shipResource.GetBoundingRadius(),

            hits,
            4
        );


    for (
        int i = 0;
        i < hitCount;
        ++i
    )
    {
        m_targetShield.Trigger(
            hits[i].normal
        );
    }


    // --------------------------------------------------------
    // SHIELD EFFECT
    // --------------------------------------------------------

    m_targetShield.Update(
        deltaTime
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
    // TARGET SIDEWINDER
    // --------------------------------------------------------

    sceGumMatrixMode(
        GU_MODEL
    );

    sceGumLoadIdentity();


    sceGumTranslate(
        &m_targetPosition
    );


    /*
        180 fokkal megfordítjuk,
        így a target a player felé néz.
    */
    ScePspFVector3 targetRotation =
    {
        0.0f,
        3.14159265f,
        0.0f
    };


    sceGumRotateXYZ(
        &targetRotation
    );


    m_shipTexture.Bind();

    m_shipMesh.Draw();

    m_shipTexture.Unbind();

    // --------------------------------------------------------
    // ENGINE PLUMES
    // --------------------------------------------------------

    m_sun.Disable();

    m_playerShip.ApplyModelTransform();

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

    m_targetShield.Draw(
        m_targetPosition,
        m_shipResource.GetBoundingRadius()
    );


    m_projectiles.Draw();


    // --------------------------------------------------------
    // HUD
    // --------------------------------------------------------

    float targetScreenX =
        0.0f;

    float targetScreenY =
        0.0f;

    float targetDepth =
        0.0f;


    const bool targetOnScreen =
        m_camera.ProjectWorldToScreen(
            m_targetPosition,

            targetScreenX,
            targetScreenY,
            targetDepth
        );


    m_hud.Draw(
        m_playerShip.GetEnginePower(),

        m_targetSelected,
        targetOnScreen,

        targetScreenX,
        targetScreenY,
        targetDepth
    );
}