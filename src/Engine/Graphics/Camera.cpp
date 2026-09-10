#include "Camera.h"

#include <pspgu.h>
#include <pspgum.h>


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