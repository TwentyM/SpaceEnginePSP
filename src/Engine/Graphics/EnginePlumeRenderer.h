#pragma once

#include <psptypes.h>


class EnginePlumeRenderer
{
public:
    void Draw(
        const ScePspFVector3& position,
        const ScePspFVector3& forward,
        float power,
        float phase
    ) const;
};