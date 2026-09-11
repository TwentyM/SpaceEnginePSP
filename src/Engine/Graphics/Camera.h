#pragma once

#include <psptypes.h>


class Camera
{
public:
    Camera();


    void Follow(
        const ScePspFVector3& position,
        const ScePspFQuaternion& orientation
    );


    void Apply() const;


    ScePspFVector3 GetPosition() const;


    bool ProjectWorldToScreen(
        const ScePspFVector3& worldPosition,
        float& screenX,
        float& screenY,
        float& depth
    ) const;


private:
    ScePspFVector3 m_position;

    ScePspFVector3 m_target;

    ScePspFVector3 m_up;


    float m_chaseDistance;

    float m_chaseHeight;

    float m_lookAhead;
};