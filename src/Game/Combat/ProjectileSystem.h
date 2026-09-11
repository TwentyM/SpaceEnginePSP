#pragma once

#include <psptypes.h>


struct ProjectileHit
{
    ScePspFVector3 position;
    ScePspFVector3 normal;
};


class ProjectileSystem
{
public:
    static constexpr int MaxProjectiles = 32;


    ProjectileSystem();


    void Spawn(
        const ScePspFVector3& position,
        const ScePspFVector3& direction
    );


    void Update(
        float deltaTime
    );


    int CheckSphereCollisions(
        const ScePspFVector3& center,
        float radius,
        ProjectileHit* hits,
        int maxHits
    );


    void Draw() const;


private:
    struct Projectile
    {
        bool active;

        ScePspFVector3 previousPosition;
        ScePspFVector3 position;
        ScePspFVector3 direction;

        float speed;

        float age;
        float lifetime;
    };


    Projectile m_projectiles[
        MaxProjectiles
    ];
};