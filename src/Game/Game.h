#pragma once

#include "../Engine/Graphics/Camera.h"
#include "../Engine/Graphics/Mesh.h"
#include "../Engine/Graphics/DirectionalLight.h"
#include "../Engine/Graphics/Starfield.h"
#include "../Engine/Resources/PspMeshResource.h"

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

    DirectionalLight m_sun;

    Starfield m_starfield;

    float m_shipRotation;
};