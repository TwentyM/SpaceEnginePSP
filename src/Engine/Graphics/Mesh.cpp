#include "Mesh.h"

#include <pspgum.h>

Mesh::Mesh()
    : m_vertices(nullptr),
      m_vertexCount(0),
      m_primitiveType(0),
      m_vertexFormat(0)
{
}

void Mesh::SetData(
    const void* vertices,
    int vertexCount,
    int primitiveType,
    int vertexFormat
)
{
    m_vertices = vertices;
    m_vertexCount = vertexCount;
    m_primitiveType = primitiveType;
    m_vertexFormat = vertexFormat;
}

void Mesh::Draw() const
{
    if (!IsValid())
    {
        return;
    }

    sceGumDrawArray(
        m_primitiveType,
        m_vertexFormat,
        m_vertexCount,
        nullptr,
        m_vertices
    );
}

bool Mesh::IsValid() const
{
    return
        m_vertices != nullptr &&
        m_vertexCount > 0;
}