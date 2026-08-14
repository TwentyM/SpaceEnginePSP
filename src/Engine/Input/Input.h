#pragma once

#include <pspctrl.h>

class Input
{
public:
    void Initialize();
    void Update();

    bool IsDown(unsigned int button) const;
    bool IsPressed(unsigned int button) const;

    float AnalogX() const;
    float AnalogY() const;

private:
    static float NormalizeAxis(unsigned char value);

private:
    SceCtrlData m_current{};
    SceCtrlData m_previous{};
};