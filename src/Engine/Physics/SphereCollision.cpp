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

bool SphereCollision::Raycast(
    const ScePspFVector3& origin,
    const ScePspFVector3& direction,

    const ScePspFVector3& center,
    float radius,

    float maxDistance,

    ScePspFVector3& hitPoint,
    float& hitDistance
)
{
    const float directionLength =
        std::sqrt(
            Dot(
                direction,
                direction
            )
        );


    if (directionLength < 0.00001f)
    {
        return false;
    }


    const float inverseLength =
        1.0f /
        directionLength;


    const ScePspFVector3 d =
    {
        direction.x *
            inverseLength,

        direction.y *
            inverseLength,

        direction.z *
            inverseLength
    };


    const ScePspFVector3 m =
        Subtract(
            origin,
            center
        );


    const float b =
        Dot(
            m,
            d
        );


    const float c =
        Dot(
            m,
            m
        ) -
        radius *
        radius;


    if (
        c > 0.0f &&
        b > 0.0f
    )
    {
        return false;
    }


    const float discriminant =
        b * b -
        c;


    if (discriminant < 0.0f)
    {
        return false;
    }


    float distance =
        -b -
        std::sqrt(
            discriminant
        );


    if (distance < 0.0f)
    {
        distance =
            0.0f;
    }


    if (
        distance >
        maxDistance
    )
    {
        return false;
    }


    hitDistance =
        distance;


    hitPoint =
    {
        origin.x +
            d.x *
            distance,

        origin.y +
            d.y *
            distance,

        origin.z +
            d.z *
            distance
    };


    return true;
}