#pragma once


struct HudWeaponReticle
{
    bool visible;

    float x;
    float y;

    bool impact;
};


struct HudTargetMarker
{
    bool selected;
    bool visible;

    float x;
    float y;
    float depth;

    bool directionValid;

    float directionX;
    float directionY;
};


struct HudFrameData
{
    float throttle;


    HudTargetMarker target;


    static constexpr int MaxWeaponReticles =
        4;


    HudWeaponReticle weaponReticles[
        MaxWeaponReticles
    ];


    int weaponReticleCount;
};


class HudRenderer
{
public:
    void Draw(
        const HudFrameData& data
    ) const;


private:
    void DrawThrottle(
        float throttle
    ) const;


    void DrawTargetTriangle(
        const HudTargetMarker& target
    ) const;


    void DrawWeaponReticle(
        const HudWeaponReticle& reticle
    ) const;


    void DrawLine(
        short x1,
        short y1,

        short x2,
        short y2,

        unsigned int color
    ) const;
};