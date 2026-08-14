#include "DirectionalLight.h"

#include <pspgu.h>

#include <cmath>

DirectionalLight::DirectionalLight()
{
    SetDirection(
        -0.55f,
        -0.35f,
        -0.75f
    );

    m_globalAmbient = 0xFF202020;
    m_ambient = 0xFF181818;
    m_diffuse = 0xFFFFFFFF;
}

void DirectionalLight::SetDirection(
    float x,
    float y,
    float z
)
{
    const float length =
        std::sqrt(
            x * x +
            y * y +
            z * z
        );

    if (length <= 0.0001f)
    {
        return;
    }

    m_direction.x = x / length;
    m_direction.y = y / length;
    m_direction.z = z / length;
}

void DirectionalLight::SetGlobalAmbient(
    unsigned int color
)
{
    m_globalAmbient = color;
}

void DirectionalLight::SetAmbient(
    unsigned int color
)
{
    m_ambient = color;
}

void DirectionalLight::SetDiffuse(
    unsigned int color
)
{
    m_diffuse = color;
}

void DirectionalLight::Apply() const
{
    sceGuEnable(GU_LIGHTING);
    sceGuEnable(GU_LIGHT0);

    sceGuLightMode(
        GU_SINGLE_COLOR
    );

    sceGuAmbient(
        m_globalAmbient
    );

    sceGuLight(
        0,
        GU_DIRECTIONAL,
        GU_AMBIENT_AND_DIFFUSE,
        &m_direction
    );

    sceGuLightColor(
        0,
        GU_AMBIENT,
        m_ambient
    );

    sceGuLightColor(
        0,
        GU_DIFFUSE,
        m_diffuse
    );
}

void DirectionalLight::Disable() const
{
    sceGuDisable(GU_LIGHT0);
    sceGuDisable(GU_LIGHTING);
}