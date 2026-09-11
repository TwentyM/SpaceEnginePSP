#include "SphereCollision.h"

#include <cmath>


namespace
{
    ScePspFVector3 Subtract(
        const ScePspFVector3& a,
        const ScePspFVector3& b
    )
    {
        return
        {
            a.x - b.x,
            a.y - b.y,
            a.z - b.z
        };
    }


    ScePspFVector3 Add(
        const ScePspFVector3& a,
        const ScePspFVector3& b
    )
    {
        return
        {
            a.x + b.x,
            a.y + b.y,
            a.z + b.z
        };
    }


    ScePspFVector3 Multiply(
        const ScePspFVector3& value,
        float scalar
    )
    {
        return
        {
            value.x * scalar,
            value.y * scalar,
            value.z * scalar
        };
    }


    float Dot(
        const ScePspFVector3& a,
        const ScePspFVector3& b
    )
    {
        return
            a.x * b.x +
            a.y * b.y +
            a.z * b.z;
    }
}


bool SphereCollision::Test(
    const ScePspFVector3& centerA,
    float radiusA,

    const ScePspFVector3& centerB,
    float radiusB,

    SphereCollisionResult& result
)
{
    const ScePspFVector3 delta =
        Subtract(
            centerA,
            centerB
        );


    const float distanceSquared =
        Dot(
            delta,
            delta
        );


    const float minimumDistance =
        radiusA +
        radiusB;


    const float minimumDistanceSquared =
        minimumDistance *
        minimumDistance;


    if (
        distanceSquared >=
        minimumDistanceSquared
    )
    {
        return false;
    }


    float distance =
        std::sqrt(
            distanceSquared
        );


    /*
        Ha a két center pontosan
        ugyanoda kerülne, kell egy
        stabil fallback normal.
    */
    if (distance < 0.0001f)
    {
        result.normal =
        {
            0.0f,
            0.0f,
            1.0f
        };


        result.penetration =
            minimumDistance;
    }
    else
    {
        const float inverseDistance =
            1.0f /
            distance;


        result.normal =
        {
            delta.x *
                inverseDistance,

            delta.y *
                inverseDistance,

            delta.z *
                inverseDistance
        };


        result.penetration =
            minimumDistance -
            distance;
    }


    /*
        A B gömb felületén levő
        érintkezési pont.

        Jelen esetben B a target hajó.
    */
    result.contactPoint =
        Add(
            centerB,
            Multiply(
                result.normal,
                radiusB
            )
        );


    return true;
}