#include "PlayerShip.h"

#include "../../Engine/Input/Input.h"

#include <pspctrl.h>
#include <pspgu.h>
#include <pspgum.h>

#include <cmath>


namespace
{
    ScePspFQuaternion CreateAxisAngle(
        float x,
        float y,
        float z,
        float angle
    )
    {
        const float halfAngle =
            angle * 0.5f;


        const float sine =
            std::sin(
                halfAngle
            );


        const float cosine =
            std::cos(
                halfAngle
            );


        return
        {
            x * sine,
            y * sine,
            z * sine,
            cosine
        };
    }


    ScePspFQuaternion Multiply(
        const ScePspFQuaternion& a,
        const ScePspFQuaternion& b
    )
    {
        ScePspFQuaternion result;


        result.x =
            a.w * b.x +
            a.x * b.w +
            a.y * b.z -
            a.z * b.y;


        result.y =
            a.w * b.y -
            a.x * b.z +
            a.y * b.w +
            a.z * b.x;


        result.z =
            a.w * b.z +
            a.x * b.y -
            a.y * b.x +
            a.z * b.w;


        result.w =
            a.w * b.w -
            a.x * b.x -
            a.y * b.y -
            a.z * b.z;


        return result;
    }


    void AddMovement(
        ScePspFVector3& position,
        const ScePspFVector3& direction,
        float amount
    )
    {
        position.x +=
            direction.x * amount;

        position.y +=
            direction.y * amount;

        position.z +=
            direction.z * amount;
    }
}


PlayerShip::PlayerShip()
    : m_moveSpeed(15.0f),
      m_strafeSpeed(7.0f),
      m_lookSpeed(1.8f),
      m_rollSpeed(1.5f),
      m_enginePower(0.25f)
{
    Reset();
}


void PlayerShip::Reset()
{
    m_position =
    {
        0.0f,
        0.0f,
        0.0f
    };


    m_orientation =
    {
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };


    m_enginePower =
        0.25f;
}


void PlayerShip::ApplyLocalRotation(
    float pitch,
    float yaw,
    float roll
)
{
    if (yaw != 0.0f)
    {
        const ScePspFQuaternion q =
            CreateAxisAngle(
                0.0f,
                1.0f,
                0.0f,
                yaw
            );


        m_orientation =
            Multiply(
                m_orientation,
                q
            );
    }


    if (pitch != 0.0f)
    {
        const ScePspFQuaternion q =
            CreateAxisAngle(
                1.0f,
                0.0f,
                0.0f,
                pitch
            );


        m_orientation =
            Multiply(
                m_orientation,
                q
            );
    }


    if (roll != 0.0f)
    {
        /*
            A hajó előre -Z felé néz,
            tehát ez a roll tengelye is.
        */
        const ScePspFQuaternion q =
            CreateAxisAngle(
                0.0f,
                0.0f,
                -1.0f,
                roll
            );


        m_orientation =
            Multiply(
                m_orientation,
                q
            );
    }


    gumNormalizeQuaternion(
        &m_orientation
    );
}


void PlayerShip::GetBasisVectors(
    ScePspFVector3& forward,
    ScePspFVector3& right,
    ScePspFVector3& up
) const
{
    const ScePspFVector3 localForward =
    {
        0.0f,
        0.0f,
        -1.0f
    };


    const ScePspFVector3 localRight =
    {
        1.0f,
        0.0f,
        0.0f
    };


    const ScePspFVector3 localUp =
    {
        0.0f,
        1.0f,
        0.0f
    };


    gumRotateVector(
        &forward,
        &m_orientation,
        &localForward
    );


    gumRotateVector(
        &right,
        &m_orientation,
        &localRight
    );


    gumRotateVector(
        &up,
        &m_orientation,
        &localUp
    );
}


void PlayerShip::Update(
    const Input& input,
    float deltaTime
)
{
    // --------------------------------------------------------
    // ROTATION
    // --------------------------------------------------------

    const float yaw =
        -input.AnalogX() *
        m_lookSpeed *
        deltaTime;


    const float pitch =
        -input.AnalogY() *
        m_lookSpeed *
        deltaTime;


    float roll =
        0.0f;


    if (
        input.IsDown(
            PSP_CTRL_LTRIGGER
        )
    )
    {
        roll +=
            m_rollSpeed *
            deltaTime;
    }


    if (
        input.IsDown(
            PSP_CTRL_RTRIGGER
        )
    )
    {
        roll -=
            m_rollSpeed *
            deltaTime;
    }


    ApplyLocalRotation(
        pitch,
        yaw,
        roll
    );


    // --------------------------------------------------------
    // LOCAL AXES
    // --------------------------------------------------------

    ScePspFVector3 forward;
    ScePspFVector3 right;
    ScePspFVector3 up;


    GetBasisVectors(
        forward,
        right,
        up
    );


    // --------------------------------------------------------
    // MOVEMENT
    // --------------------------------------------------------

    float forwardSpeed =
        m_moveSpeed;


    const bool boost =
        input.IsDown(
            PSP_CTRL_CIRCLE
        );


    if (boost)
    {
        forwardSpeed *=
            2.5f;
    }


    if (
        input.IsDown(
            PSP_CTRL_TRIANGLE
        )
    )
    {
        AddMovement(
            m_position,
            forward,
            forwardSpeed *
            deltaTime
        );
    }


    if (
        input.IsDown(
            PSP_CTRL_CROSS
        )
    )
    {
        AddMovement(
            m_position,
            forward,
            -m_moveSpeed *
            deltaTime
        );
    }


    const float strafeMovement =
        m_strafeSpeed *
        deltaTime;


    if (
        input.IsDown(
            PSP_CTRL_RIGHT
        )
    )
    {
        AddMovement(
            m_position,
            right,
            strafeMovement
        );
    }


    if (
        input.IsDown(
            PSP_CTRL_LEFT
        )
    )
    {
        AddMovement(
            m_position,
            right,
            -strafeMovement
        );
    }


    if (
        input.IsDown(
            PSP_CTRL_UP
        )
    )
    {
        AddMovement(
            m_position,
            up,
            strafeMovement
        );
    }


    if (
        input.IsDown(
            PSP_CTRL_DOWN
        )
    )
    {
        AddMovement(
            m_position,
            up,
            -strafeMovement
        );
    }


    // --------------------------------------------------------
    // ENGINE POWER
    // --------------------------------------------------------

    float targetPower =
        0.25f;


    if (
        input.IsDown(
            PSP_CTRL_TRIANGLE
        )
    )
    {
        targetPower =
            boost
                ? 1.0f
                : 0.70f;
    }


    if (
        input.IsDown(
            PSP_CTRL_CROSS
        )
    )
    {
        targetPower =
            0.40f;
    }


    float response =
        deltaTime *
        6.0f;


    if (response > 1.0f)
    {
        response = 1.0f;
    }


    m_enginePower +=
        (
            targetPower -
            m_enginePower
        ) *
        response;


    // --------------------------------------------------------
    // RESET
    // --------------------------------------------------------

    if (
        input.IsPressed(
            PSP_CTRL_SQUARE
        )
    )
    {
        Reset();
    }
}


void PlayerShip::ApplyModelTransform() const
{
    sceGumMatrixMode(
        GU_MODEL
    );


    sceGumLoadIdentity();


    sceGumTranslate(
        &m_position
    );


    sceGumRotate(
        &m_orientation
    );
}


ScePspFVector3
PlayerShip::GetPosition() const
{
    return m_position;
}


ScePspFQuaternion
PlayerShip::GetOrientation() const
{
    return m_orientation;
}


float PlayerShip::GetEnginePower() const
{
    return m_enginePower;
}

void PlayerShip::AddWorldOffset(
    const ScePspFVector3& offset
)
{
    m_position.x +=
        offset.x;

    m_position.y +=
        offset.y;

    m_position.z +=
        offset.z;
}