#pragma once

#include <psptypes.h>


class ProjectileSystem
{
public:
    static constexpr int MaxProjectiles = 32;


    ProjectileSystem();


    void Spawn(
        const ScePspFMatrix4& spawnTransform,
        const ScePspFVector3& position,
        const ScePspFVector3& direction
    );


    void Update(
        float deltaTime
    );


    void Draw() const;


private:
    struct Projectile
    {
        bool active;

        ScePspFMatrix4 spawnTransform;

        ScePspFVector3 origin;
        ScePspFVector3 direction;

        float distance;
        float speed;

        float age;
        float lifetime;
    };


    Projectile m_projectiles[
        MaxProjectiles
    ];
};