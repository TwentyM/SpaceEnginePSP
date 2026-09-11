#pragma once


class HudRenderer
{
public:
    void Draw(
        float enginePower,

        bool targetSelected,
        bool targetOnScreen,

        float targetScreenX,
        float targetScreenY,
        float targetDepth
    ) const;


private:
    void DrawCrosshair() const;


    void DrawThrottle(
        float enginePower
    ) const;


    void DrawTargetBracket(
        float screenX,
        float screenY,
        float depth
    ) const;


    void DrawLine(
        short x1,
        short y1,

        short x2,
        short y2,

        unsigned int color
    ) const;
};