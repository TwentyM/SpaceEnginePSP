#pragma once

#include <psptypes.h>


class ShieldHitEffect
{
public:
    static constexpr int MaxHits = 4;


    ShieldHitEffect();


    void Trigger(
        const ScePspFVector3& normal
    );


    void Update(
        float deltaTime
    );


    void Draw(
        const ScePspFVector3& center,
        float radius
    ) const;


private:
    struct Hit
    {
        bool active;

        ScePspFVector3 normal;

        float age;
        float duration;
    };


    Hit m_hits[
        MaxHits
    ];
};