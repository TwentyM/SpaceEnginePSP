#include "Camera.h"

#include "../Input/Input.h"

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
        const float halfAngle = angle * 0.5f;

        const float s = std::sin(halfAngle);
        const float c = std::cos(halfAngle);

        ScePspFQuaternion result =
        {
            x * s,
            y * s,
            z * s,
            c
        };

        return result;
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
}


Camera::Camera()
{
    m_moveSpeed = 4.0f;
    m_lookSpeed = 1.8f;
    m_rollSpeed = 1.5f;

    Reset();
}


void Camera::Reset()
{
    m_x = 0.0f;
    m_y = 0.0f;
    m_z = 5.0f;

    // Identity quaternion.
    m_orientation.x = 0.0f;
    m_orientation.y = 0.0f;
    m_orientation.z = 0.0f;
    m_orientation.w = 1.0f;
}


void Camera::ApplyLocalRotation(
    float pitch,
    float yaw,
    float roll
)
{
    /*
        Ezek LOKÁLIS tengelyek:

        X = pitch
        Y = yaw
       -Z = forward / roll axis

        Mivel az aktuális orientation jobb oldalára
        szorozzuk a delta quaternionokat,
        a forgatások a kamera saját koordinátáihoz
        képest történnek.
    */


    if (yaw != 0.0f)
    {
        const ScePspFQuaternion qYaw =
            CreateAxisAngle(
                0.0f,
                1.0f,
                0.0f,
                yaw
            );

        m_orientation =
            Multiply(
                m_orientation,
                qYaw
            );
    }


    if (pitch != 0.0f)
    {
        const ScePspFQuaternion qPitch =
            CreateAxisAngle(
                1.0f,
                0.0f,
                0.0f,
                pitch
            );

        m_orientation =
            Multiply(
                m_orientation,
                qPitch
            );
    }


    if (roll != 0.0f)
    {
        // A kamera előre -Z felé néz.
        const ScePspFQuaternion qRoll =
            CreateAxisAngle(
                0.0f,
                0.0f,
                -1.0f,
                roll
            );

        m_orientation =
            Multiply(
                m_orientation,
                qRoll
            );
    }


    // A sok apró forgatás miatt idővel lebegőpontos
    // pontatlanság gyűlne össze.
    gumNormalizeQuaternion(
        &m_orientation
    );
}


void Camera::GetBasisVectors(
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


void Camera::Update(
    const Input& input,
    float deltaTime
)
{
    // --------------------------------------------------------
    // LOCAL ROTATION
    // --------------------------------------------------------

    const float analogX =
        input.AnalogX();

    const float analogY =
        input.AnalogY();


    /*
        PSP stick:

        jobbra  -> +X
        lefelé  -> +Y

        Emiatt itt megfordítjuk az irányokat.
    */

    const float yawDelta =
        -analogX *
        m_lookSpeed *
        deltaTime;

    const float pitchDelta =
        -analogY *
        m_lookSpeed *
        deltaTime;


    float rollDelta = 0.0f;

    if (input.IsDown(PSP_CTRL_LTRIGGER))
    {
        rollDelta +=
            m_rollSpeed *
            deltaTime;
    }

    if (input.IsDown(PSP_CTRL_RTRIGGER))
    {
        rollDelta -=
            m_rollSpeed *
            deltaTime;
    }


    ApplyLocalRotation(
        pitchDelta,
        yawDelta,
        rollDelta
    );


    // --------------------------------------------------------
    // LOCAL COORDINATE SYSTEM
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
    // SPEED
    // --------------------------------------------------------

    float speed =
        m_moveSpeed;

    if (input.IsDown(PSP_CTRL_CIRCLE))
    {
        speed *= 3.0f;
    }


    const float movement =
        speed *
        deltaTime;


    // --------------------------------------------------------
    // FORWARD / BACKWARD
    // --------------------------------------------------------

    if (input.IsDown(PSP_CTRL_TRIANGLE))
    {
        m_x += forward.x * movement;
        m_y += forward.y * movement;
        m_z += forward.z * movement;
    }

    if (input.IsDown(PSP_CTRL_CROSS))
    {
        m_x -= forward.x * movement;
        m_y -= forward.y * movement;
        m_z -= forward.z * movement;
    }


    // --------------------------------------------------------
    // STRAFE LEFT / RIGHT
    // --------------------------------------------------------

    if (input.IsDown(PSP_CTRL_RIGHT))
    {
        m_x += right.x * movement;
        m_y += right.y * movement;
        m_z += right.z * movement;
    }

    if (input.IsDown(PSP_CTRL_LEFT))
    {
        m_x -= right.x * movement;
        m_y -= right.y * movement;
        m_z -= right.z * movement;
    }


    // --------------------------------------------------------
    // STRAFE UP / DOWN
    // --------------------------------------------------------

    if (input.IsDown(PSP_CTRL_UP))
    {
        m_x += up.x * movement;
        m_y += up.y * movement;
        m_z += up.z * movement;
    }

    if (input.IsDown(PSP_CTRL_DOWN))
    {
        m_x -= up.x * movement;
        m_y -= up.y * movement;
        m_z -= up.z * movement;
    }


    // --------------------------------------------------------
    // RESET
    // --------------------------------------------------------

    if (input.IsPressed(PSP_CTRL_SQUARE))
    {
        Reset();
    }
}


void Camera::Apply() const
{
    // --------------------------------------------------------
    // PROJECTION
    // --------------------------------------------------------

    sceGumMatrixMode(
        GU_PROJECTION
    );

    sceGumLoadIdentity();

    sceGumPerspective(
        60.0f,
        480.0f / 272.0f,
        0.1f,
        1000.0f
    );


    // --------------------------------------------------------
    // CAMERA BASIS
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
    // VIEW MATRIX
    // --------------------------------------------------------

    ScePspFVector3 eye =
    {
        m_x,
        m_y,
        m_z
    };


    ScePspFVector3 center =
    {
        m_x + forward.x,
        m_y + forward.y,
        m_z + forward.z
    };


    sceGumMatrixMode(
        GU_VIEW
    );

    sceGumLoadIdentity();

    sceGumLookAt(
        &eye,
        &center,
        &up
    );
}