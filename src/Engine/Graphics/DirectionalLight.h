#pragma once

#include <psptypes.h>

class DirectionalLight
{
public:
    DirectionalLight();

    void SetDirection(
        float x,
        float y,
        float z
    );

    void SetGlobalAmbient(
        unsigned int color
    );

    void SetAmbient(
        unsigned int color
    );

    void SetDiffuse(
        unsigned int color
    );

    void Apply() const;
    void Disable() const;

private:
    ScePspFVector3 m_direction;

    unsigned int m_globalAmbient;
    unsigned int m_ambient;
    unsigned int m_diffuse;
};