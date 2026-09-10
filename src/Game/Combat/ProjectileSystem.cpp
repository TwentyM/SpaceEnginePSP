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
                value.x * value.x +
                value.y * value.y +
                value.z * value.z
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


        const float inverse =
            1.0f / length;


        return
        {
            value.x * inverse,
            value.y * inverse,
            value.z * inverse
        };
    }


    void WriteVertex(
        BoltVertex& vertex,
        unsigned int color,
        const ScePspFVector3& position
    )
    {
        vertex.color = color;

        vertex.x = position.x;
        vertex.y = position.y;
        vertex.z = position.z;
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
            Add(
                tail,
                Multiply(
                    offset,
                    -1.0f
                )
            );


        const ScePspFVector3 tailRight =
            Add(
                tail,
                offset
            );


        const ScePspFVector3 headLeft =
            Add(
                head,
                Multiply(
                    offset,
                    -1.0f
                )
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
        const ScePspFVector3& origin,
        const ScePspFVector3& direction,

        float distance,
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


        const ScePspFVector3 head =
            Add(
                origin,
                Multiply(
                    forward,
                    distance
                )
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


        constexpr int VertexCount = 12;


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
    const ScePspFMatrix4& spawnTransform,
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


        /*
            Eltesszük a hajó spawn pillanatbeli
            model matrixát.

            Ez nagyon fontos:
            a lövedék ezután már NEM forog
            együtt a hajóval.
        */
        projectile.spawnTransform =
            spawnTransform;


        projectile.origin =
            position;


        projectile.direction =
            Normalize(
                direction
            );


        projectile.distance =
            0.60f;


        projectile.speed =
            34.0f;


        projectile.age =
            0.0f;


        projectile.lifetime =
            2.5f;


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


        projectile.age +=
            deltaTime;


        projectile.distance +=
            projectile.speed *
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


    /*
        Ütközzenek a depth bufferrel,
        viszont a glow ne írja felül azt.
    */
    sceGuDepthMask(
        GU_TRUE
    );


    sceGumMatrixMode(
        GU_MODEL
    );


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


        /*
            A lövedék saját, spawnkor eltett
            koordinátarendszerét használjuk.
        */
        sceGumLoadMatrix(
            &projectile.spawnTransform
        );


        // Külső narancssárga glow.
        DrawLayer(
            projectile.origin,
            projectile.direction,

            projectile.distance,

            1.70f,
            0.18f,

            0x204060FF,
            0xD040A0FF
        );


        // Fényes belső mag.
        DrawLayer(
            projectile.origin,
            projectile.direction,

            projectile.distance,

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


    sceGumLoadIdentity();
}