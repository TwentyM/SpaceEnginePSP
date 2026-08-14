#pragma once

#include <psptypes.h>
#include <cstdint>

class Starfield
{
public:
    static constexpr int StarCount = 256;

    Starfield();

    void Generate(
        std::uint32_t seed
    );

    void Draw(
        const ScePspFVector3& cameraPosition
    ) const;

private:
    struct StarVertex
    {
        unsigned int color;

        float x;
        float y;
        float z;
    };

private:
    alignas(16)
    StarVertex m_vertices[StarCount];
};