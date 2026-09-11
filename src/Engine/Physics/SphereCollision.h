#pragma once

#include <psptypes.h>


struct SphereCollisionResult
{
    ScePspFVector3 normal;

    ScePspFVector3 contactPoint;

    float penetration;
};


class SphereCollision
{
public:
    static bool Test(
        const ScePspFVector3& centerA,
        float radiusA,

        const ScePspFVector3& centerB,
        float radiusB,

        SphereCollisionResult& result
    );
};