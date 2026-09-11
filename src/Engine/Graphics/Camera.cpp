#include "Camera.h"

#include <pspgu.h>
#include <pspgum.h>
#include <cmath>
namespace
{
    constexpr float Pi =
        3.14159265358979323846f;


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


        const float inverse =
            1.0f /
            length;


        return
        {
            value.x * inverse,
            value.y * inverse,
            value.z * inverse
        };
    }
}

Camera::Camera()
    : m_chaseDistance(32.0f),
      m_chaseHeight(7.0f),
      m_lookAhead(4.0f)
{
    m_position =
    {
        0.0f,
        7.0f,
        32.0f
    };


    m_target =
    {
        0.0f,
        0.0f,
        -4.0f
    };


    m_up =
    {
        0.0f,
        1.0f,
        0.0f
    };
}


void Camera::Follow(
    const ScePspFVector3& position,
    const ScePspFQuaternion& orientation
)
{
    const ScePspFVector3 localForward =
    {
        0.0f,
        0.0f,
        -1.0f
    };


    const ScePspFVector3 localUp =
    {
        0.0f,
        1.0f,
        0.0f
    };


    ScePspFVector3 forward;
    ScePspFVector3 up;


    gumRotateVector(
        &forward,
        &orientation,
        &localForward
    );


    gumRotateVector(
        &up,
        &orientation,
        &localUp
    );


    /*
        Kamera a hajó mögött és kicsivel
        fölötte helyezkedik el.
    */
    m_position.x =
        position.x -
        forward.x *
        m_chaseDistance +
        up.x *
        m_chaseHeight;


    m_position.y =
        position.y -
        forward.y *
        m_chaseDistance +
        up.y *
        m_chaseHeight;


    m_position.z =
        position.z -
        forward.z *
        m_chaseDistance +
        up.z *
        m_chaseHeight;


    /*
        Nem pontosan a hajó közepére nézünk,
        hanem néhány méterrel elé.
    */
    m_target.x =
        position.x +
        forward.x *
        m_lookAhead;


    m_target.y =
        position.y +
        forward.y *
        m_lookAhead;


    m_target.z =
        position.z +
        forward.z *
        m_lookAhead;


    m_up =
        up;
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
        2000.0f
    );


    // --------------------------------------------------------
    // VIEW
    // --------------------------------------------------------

    ScePspFVector3 eye =
        m_position;


    ScePspFVector3 target =
        m_target;


    ScePspFVector3 up =
        m_up;


    sceGumMatrixMode(
        GU_VIEW
    );


    sceGumLoadIdentity();


    sceGumLookAt(
        &eye,
        &target,
        &up
    );
}


ScePspFVector3
Camera::GetPosition() const
{
    return m_position;
}


bool Camera::ProjectWorldToScreen(
    const ScePspFVector3& worldPosition,
    float& screenX,
    float& screenY,
    float& depth
) const
{
    const ScePspFVector3 forward =
        Normalize(
            Subtract(
                m_target,
                m_position
            )
        );


    const ScePspFVector3 right =
        Normalize(
            Cross(
                forward,
                m_up
            )
        );


    const ScePspFVector3 up =
        Normalize(
            Cross(
                right,
                forward
            )
        );


    const ScePspFVector3 relative =
        Subtract(
            worldPosition,
            m_position
        );


    depth =
        Dot(
            relative,
            forward
        );


    /*
        Kamera mögött vagy túl közel.
    */
    if (depth <= 0.1f)
    {
        return false;
    }


    const float cameraX =
        Dot(
            relative,
            right
        );


    const float cameraY =
        Dot(
            relative,
            up
        );


    constexpr float fovYDegrees =
        60.0f;


    const float halfFovRadians =
        (
            fovYDegrees *
            0.5f
        ) *
        (
            Pi /
            180.0f
        );


    const float tanHalfFov =
        std::tan(
            halfFovRadians
        );


    constexpr float screenWidth =
        480.0f;

    constexpr float screenHeight =
        272.0f;

    constexpr float aspect =
        screenWidth /
        screenHeight;


    const float ndcX =
        cameraX /
        (
            depth *
            tanHalfFov *
            aspect
        );


    const float ndcY =
        cameraY /
        (
            depth *
            tanHalfFov
        );


    screenX =
        (
            ndcX *
            0.5f +
            0.5f
        ) *
        screenWidth;


    screenY =
        (
            0.5f -
            ndcY *
            0.5f
        ) *
        screenHeight;


    return
        ndcX >= -1.0f &&
        ndcX <= 1.0f &&
        ndcY >= -1.0f &&
        ndcY <= 1.0f;
}