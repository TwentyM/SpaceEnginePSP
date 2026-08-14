#include "Input.h"

#include <cmath>

namespace
{
    constexpr float ANALOG_DEADZONE = 0.15f;
}

void Input::Initialize()
{
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    sceCtrlPeekBufferPositive(&m_current, 1);
    m_previous = m_current;
}

void Input::Update()
{
    m_previous = m_current;

    sceCtrlPeekBufferPositive(
        &m_current,
        1
    );
}

bool Input::IsDown(unsigned int button) const
{
    return (m_current.Buttons & button) != 0;
}

bool Input::IsPressed(unsigned int button) const
{
    const bool current =
        (m_current.Buttons & button) != 0;

    const bool previous =
        (m_previous.Buttons & button) != 0;

    return current && !previous;
}

float Input::AnalogX() const
{
    return NormalizeAxis(m_current.Lx);
}

float Input::AnalogY() const
{
    return NormalizeAxis(m_current.Ly);
}

float Input::NormalizeAxis(unsigned char value)
{
    float normalized;

    if (value >= 128)
    {
        normalized =
            static_cast<float>(value - 128) / 127.0f;
    }
    else
    {
        normalized =
            static_cast<float>(value - 128) / 128.0f;
    }

    const float absValue = std::fabs(normalized);

    if (absValue < ANALOG_DEADZONE)
    {
        return 0.0f;
    }

    // A deadzone után újra 0..1 tartományra húzzuk.
    const float scaled =
        (absValue - ANALOG_DEADZONE) /
        (1.0f - ANALOG_DEADZONE);

    return normalized < 0.0f
        ? -scaled
        : scaled;
}