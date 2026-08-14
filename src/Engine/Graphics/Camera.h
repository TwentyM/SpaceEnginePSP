#pragma once

#include <psptypes.h>

class Input;

class Camera
{
public:
    Camera();

    void Update(
        const Input& input,
        float deltaTime
    );

    void Apply() const;

    void Reset();

private:
    void GetBasisVectors(
        ScePspFVector3& forward,
        ScePspFVector3& right,
        ScePspFVector3& up
    ) const;

    void ApplyLocalRotation(
        float pitch,
        float yaw,
        float roll
    );

private:
    float m_x;
    float m_y;
    float m_z;

    ScePspFQuaternion m_orientation;

    float m_moveSpeed;
    float m_lookSpeed;
    float m_rollSpeed;
};