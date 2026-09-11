#pragma once

#include "../Engine/Graphics/Camera.h"
#include "../Engine/Graphics/Mesh.h"
#include "../Engine/Graphics/DirectionalLight.h"
#include "../Engine/Graphics/Starfield.h"
#include "../Engine/Resources/PspMeshResource.h"
#include "../Engine/Resources/PspTextureResource.h"
#include "../Engine/Graphics/EnginePlumeRenderer.h"
#include "Combat/ProjectileSystem.h"
#include "Ships/PlayerShip.h"
#include "Combat/ShieldHitEffect.h"

class Input;
class Renderer;

class Game
{
public:
    Game();

    void Initialize();

    void Update(
        const Input& input,
        float deltaTime
    );

    void Render(
        Renderer& renderer
    );

private:
    Camera m_camera;

    Mesh m_shipMesh;
    PspMeshResource m_shipResource;
    PspTextureResource m_shipTexture;

    DirectionalLight m_sun;

    Starfield m_starfield;

    EnginePlumeRenderer m_enginePlume;

    ProjectileSystem m_projectiles;

    PlayerShip m_playerShip;

    ShieldHitEffect m_targetShield;

    ScePspFVector3 m_targetPosition;

    float m_enginePhase;

    bool m_playerTouchingTarget;
};