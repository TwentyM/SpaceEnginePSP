#pragma once

class Mesh
{
public:
    Mesh();

    void SetData(
        const void* vertices,
        int vertexCount,
        int primitiveType,
        int vertexFormat
    );

    void Draw() const;

    bool IsValid() const;

private:
    const void* m_vertices;

    int m_vertexCount;
    int m_primitiveType;
    int m_vertexFormat;
};