#pragma once

#include <psptypes.h>

#include <cstdint>

class Mesh;


enum class PspMeshMarkerType : std::uint32_t
{
    Engine = 1,
    Weapon = 2
};


struct PspMeshMarker
{
    PspMeshMarkerType type;

    char name[32];

    ScePspFVector3 position;
    ScePspFVector3 forward;
};


struct PspMeshVertex
{
    float u;
    float v;

    float nx;
    float ny;
    float nz;

    float x;
    float y;
    float z;
};


class PspMeshResource
{
public:
    PspMeshResource();

    ~PspMeshResource();

    PspMeshResource(
        const PspMeshResource&
    ) = delete;

    PspMeshResource& operator=(
        const PspMeshResource&
    ) = delete;


    bool Load(
        const char* path
    );

    void Unload();

    bool IsLoaded() const;


    void BindTo(
        Mesh& mesh
    ) const;


    float GetBoundingRadius() const;


    int GetMarkerCount() const;

    const PspMeshMarker* GetMarker(
        int index
    ) const;


private:
    PspMeshVertex* m_vertices;

    PspMeshMarker* m_markers;

    int m_vertexCount;
    int m_markerCount;

    float m_boundingRadius;
};