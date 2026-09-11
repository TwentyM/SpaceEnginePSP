#pragma once

#include <psptypes.h>


class Input;


class PlayerShip
{
public:
    PlayerShip();


    void Update(
        const Input& input,
        float deltaTime
    );


    void Reset();

    void AddWorldOffset(
        const ScePspFVector3& offset
    );

    void ApplyModelTransform() const;


    ScePspFVector3 GetPosition() const;

    ScePspFQuaternion GetOrientation() const;


    float GetEnginePower() const;


private:
    void ApplyLocalRotation(
        float pitch,
        float yaw,
        float roll
    );


    void GetBasisVectors(
        ScePspFVector3& forward,
        ScePspFVector3& right,
        ScePspFVector3& up
    ) const;


private:
    ScePspFVector3 m_position;

    ScePspFQuaternion m_orientation;


    float m_moveSpeed;
    float m_strafeSpeed;

    float m_lookSpeed;
    float m_rollSpeed;


    float m_enginePower;
};