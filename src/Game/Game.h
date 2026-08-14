#pragma once

#include "../Engine/Graphics/Camera.h"

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

    float m_cubeRotation;
};