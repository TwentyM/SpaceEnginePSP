#include "ProjectileSystem.h"

#include <pspgu.h>
#include <pspgum.h>

#include <cmath>


namespace
{
    struct BoltVertex
    {
        unsigned int color;

        float x;
        float y;
        float z;
    };


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


    ScePspFVector3 Cross(
        const ScePspFVector3& a,
        const ScePspFVector3& b
    )
    {
        return
        {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }


    ScePspFVector3 Normalize(
        const ScePspFVector3& value
    )
    {
        const float length =
            std::sqrt(
                Dot(
                    value,
                    value
                )
            );


        if (length < 0.00001f)
        {
            return
            {
                0.0f,
                0.0f,
                -1.0f
            };
        }


        return Multiply(
            value,
            1.0f / length
        );
    }


    bool SegmentSphereIntersection(
        const ScePspFVector3& start,
        const ScePspFVector3& end,

        const ScePspFVector3& center,
        float radius,

        ScePspFVector3& hitPosition
    )
    {
        const ScePspFVector3 d =
            Subtract(
                end,
                start
            );


        const ScePspFVector3 m =
            Subtract(
                start,
                center
            );


        const float a =
            Dot(
                d,
                d
            );


        if (a < 0.000001f)
        {
            return false;
        }


        const float b =
            2.0f *
            Dot(
                m,
                d
            );


        const float c =
            Dot(
                m,
                m
            ) -
            radius * radius;


        const float discriminant =
            b * b -
            4.0f * a * c;


        if (discriminant < 0.0f)
        {
            return false;
        }


        const float root =
            std::sqrt(
                discriminant
            );


        const float inverse =
            1.0f /
            (
                2.0f * a
            );


        float t =
            (
                -b - root
            ) *
            inverse;


        if (
            t < 0.0f ||
            t > 1.0f
        )
        {
            t =
                (
                    -b + root
                ) *
                inverse;
        }


        if (
            t < 0.0f ||
            t > 1.0f
        )
        {
            return false;
        }


        hitPosition =
            Add(
                start,
                Multiply(
                    d,
                    t
                )
            );


        return true;
    }


    void WriteVertex(
        BoltVertex& vertex,
        unsigned int color,
        const ScePspFVector3& position
    )
    {
        vertex.color =
            color;

        vertex.x =
            position.x;

        vertex.y =
            position.y;

        vertex.z =
            position.z;
    }


    void WriteQuad(
        BoltVertex* vertices,

        const ScePspFVector3& tail,
        const ScePspFVector3& head,

        const ScePspFVector3& axis,

        float width,

        unsigned int tailColor,
        unsigned int headColor
    )
    {
        const ScePspFVector3 offset =
            Multiply(
                axis,
                width
            );


        const ScePspFVector3 tailLeft =
            Subtract(
                tail,
                offset
            );


        const ScePspFVector3 tailRight =
            Add(
                tail,
                offset
            );


        const ScePspFVector3 headLeft =
            Subtract(
                head,
                offset
            );


        const ScePspFVector3 headRight =
            Add(
                head,
                offset
            );


        WriteVertex(
            vertices[0],
            tailColor,
            tailLeft
        );

        WriteVertex(
            vertices[1],
            tailColor,
            tailRight
        );

        WriteVertex(
            vertices[2],
            headColor,
            headRight
        );


        WriteVertex(
            vertices[3],
            tailColor,
            tailLeft
        );

        WriteVertex(
            vertices[4],
            headColor,
            headRight
        );

        WriteVertex(
            vertices[5],
            headColor,
            headLeft
        );
    }


    void DrawLayer(
        const ScePspFVector3& head,
        const ScePspFVector3& direction,

        float length,
        float width,

        unsigned int tailColor,
        unsigned int headColor
    )
    {
        const ScePspFVector3 forward =
            Normalize(
                direction
            );


        const ScePspFVector3 tail =
            Add(
                head,
                Multiply(
                    forward,
                    -length
                )
            );


        ScePspFVector3 reference =
        {
            0.0f,
            1.0f,
            0.0f
        };


        if (
            std::fabs(
                forward.y
            ) > 0.90f
        )
        {
            reference =
            {
                1.0f,
                0.0f,
                0.0f
            };
        }


        const ScePspFVector3 axisA =
            Normalize(
                Cross(
                    forward,
                    reference
                )
            );


        const ScePspFVector3 axisB =
            Normalize(
                Cross(
                    forward,
                    axisA
                )
            );


        constexpr int VertexCount =
            12;


        BoltVertex* vertices =
            static_cast<BoltVertex*>(
                sceGuGetMemory(
                    sizeof(BoltVertex) *
                    VertexCount
                )
            );


        if (vertices == nullptr)
        {
            return;
        }


        WriteQuad(
            &vertices[0],

            tail,
            head,

            axisA,

            width,

            tailColor,
            headColor
        );


        WriteQuad(
            &vertices[6],

            tail,
            head,

            axisB,

            width,

            tailColor,
            headColor
        );


        sceGumDrawArray(
            GU_TRIANGLES,

            GU_COLOR_8888 |
            GU_VERTEX_32BITF |
            GU_TRANSFORM_3D,

            VertexCount,

            nullptr,
            vertices
        );
    }
}


ProjectileSystem::ProjectileSystem()
{
    for (
        int i = 0;
        i < MaxProjectiles;
        ++i
    )
    {
        m_projectiles[i].active =
            false;
    }
}


void ProjectileSystem::Spawn(
    const ScePspFVector3& position,
    const ScePspFVector3& direction
)
{
    for (
        int i = 0;
        i < MaxProjectiles;
        ++i
    )
    {
        Projectile& projectile =
            m_projectiles[i];


        if (projectile.active)
        {
            continue;
        }


        projectile.active =
            true;


        projectile.direction =
            Normalize(
                direction
            );


        /*
            Picit előrébb indul,
            nehogy saját hajótestből
            induljon.
        */
        projectile.position =
            Add(
                position,
                Multiply(
                    projectile.direction,
                    0.6f
                )
            );


        projectile.previousPosition =
            projectile.position;


        projectile.speed =
            55.0f;


        projectile.age =
            0.0f;


        projectile.lifetime =
            3.0f;


        return;
    }
}


void ProjectileSystem::Update(
    float deltaTime
)
{
    for (
        int i = 0;
        i < MaxProjectiles;
        ++i
    )
    {
        Projectile& projectile =
            m_projectiles[i];


        if (!projectile.active)
        {
            continue;
        }


        projectile.previousPosition =
            projectile.position;


        projectile.position =
            Add(
                projectile.position,
                Multiply(
                    projectile.direction,
                    projectile.speed *
                    deltaTime
                )
            );


        projectile.age +=
            deltaTime;


        if (
            projectile.age >=
            projectile.lifetime
        )
        {
            projectile.active =
                false;
        }
    }
}


int ProjectileSystem::CheckSphereCollisions(
    const ScePspFVector3& center,
    float radius,
    ProjectileHit* hits,
    int maxHits
)
{
    if (
        hits == nullptr ||
        maxHits <= 0
    )
    {
        return 0;
    }


    int hitCount =
        0;


    for (
        int i = 0;
        i < MaxProjectiles;
        ++i
    )
    {
        Projectile& projectile =
            m_projectiles[i];


        if (!projectile.active)
        {
            continue;
        }


        ScePspFVector3 hitPosition;


        if (
            !SegmentSphereIntersection(
                projectile.previousPosition,
                projectile.position,

                center,
                radius,

                hitPosition
            )
        )
        {
            continue;
        }


        projectile.active =
            false;


        ProjectileHit& hit =
            hits[hitCount];


        hit.position =
            hitPosition;


        hit.normal =
            Normalize(
                Subtract(
                    hitPosition,
                    center
                )
            );


        ++hitCount;


        if (
            hitCount >=
            maxHits
        )
        {
            break;
        }
    }


    return hitCount;
}


void ProjectileSystem::Draw() const
{
    sceGuDisable(
        GU_LIGHTING
    );

    sceGuDisable(
        GU_LIGHT0
    );

    sceGuDisable(
        GU_TEXTURE_2D
    );


    sceGuEnable(
        GU_BLEND
    );


    sceGuBlendFunc(
        GU_ADD,
        GU_SRC_ALPHA,
        GU_FIX,
        0,
        0xFFFFFFFF
    );


    sceGuDepthMask(
        GU_TRUE
    );


    /*
        A projectile-ok már világkoordinátában
        vannak.
    */
    sceGumMatrixMode(
        GU_MODEL
    );

    sceGumLoadIdentity();


    for (
        int i = 0;
        i < MaxProjectiles;
        ++i
    )
    {
        const Projectile& projectile =
            m_projectiles[i];


        if (!projectile.active)
        {
            continue;
        }


        DrawLayer(
            projectile.position,
            projectile.direction,

            1.70f,
            0.18f,

            0x204060FF,
            0xD040A0FF
        );


        DrawLayer(
            projectile.position,
            projectile.direction,

            1.25f,
            0.075f,

            0x20FFFFFF,
            0xFFFFFFFF
        );
    }


    sceGuDepthMask(
        GU_FALSE
    );


    sceGuDisable(
        GU_BLEND
    );
}