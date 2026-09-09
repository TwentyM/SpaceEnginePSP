#include "PspMeshResource.h"

#include "../Graphics/Mesh.h"

#include <pspiofilemgr.h>
#include <pspkernel.h>
#include <pspgu.h>

#include <malloc.h>

#include <cstdlib>
#include <cstring>


namespace
{
    constexpr std::uint32_t ExpectedVersion = 2;


#pragma pack(push, 1)

    struct FileHeader
    {
        char magic[8];

        std::uint32_t version;
        std::uint32_t vertexCount;
        std::uint32_t markerCount;

        float boundingRadius;
    };


    struct FileMarker
    {
        std::uint32_t type;

        char name[32];

        float px;
        float py;
        float pz;

        float fx;
        float fy;
        float fz;
    };

#pragma pack(pop)


    bool ReadExact(
        SceUID file,
        void* destination,
        std::size_t size
    )
    {
        const int result =
            sceIoRead(
                file,
                destination,
                static_cast<SceSize>(size)
            );

        return
            result ==
            static_cast<int>(size);
    }


    bool IsMagicValid(
        const char magic[8]
    )
    {
        static const char expected[8] =
        {
            'S',
            'E',
            'P',
            'S',
            'P',
            'M',
            '0',
            '1'
        };

        return
            std::memcmp(
                magic,
                expected,
                8
            ) == 0;
    }
}


PspMeshResource::PspMeshResource()
    : m_vertices(nullptr),
      m_markers(nullptr),
      m_vertexCount(0),
      m_markerCount(0),
      m_boundingRadius(0.0f)
{
}


PspMeshResource::~PspMeshResource()
{
    Unload();
}


bool PspMeshResource::Load(
    const char* path
)
{
    Unload();


    SceUID file =
        sceIoOpen(
            path,
            PSP_O_RDONLY,
            0777
        );


    if (file < 0)
    {
        return false;
    }


    FileHeader header{};


    if (!ReadExact(
        file,
        &header,
        sizeof(header)
    ))
    {
        sceIoClose(file);

        return false;
    }


    if (
        !IsMagicValid(
            header.magic
        ) ||
        header.version != ExpectedVersion ||
        header.vertexCount == 0 ||
        header.vertexCount > 1000000 ||
        header.markerCount > 256
    )
    {
        sceIoClose(file);

        return false;
    }


    const std::size_t vertexBytes =
        sizeof(PspMeshVertex) *
        header.vertexCount;


    PspMeshVertex* vertices =
        static_cast<PspMeshVertex*>(
            memalign(
                16,
                vertexBytes
            )
        );


    if (vertices == nullptr)
    {
        sceIoClose(file);

        return false;
    }


    if (!ReadExact(
        file,
        vertices,
        vertexBytes
    ))
    {
        std::free(vertices);

        sceIoClose(file);

        return false;
    }


    PspMeshMarker* markers = nullptr;


    if (header.markerCount > 0)
    {
        markers =
            new PspMeshMarker[
                header.markerCount
            ];


        for (
            std::uint32_t i = 0;
            i < header.markerCount;
            ++i
        )
        {
            FileMarker fileMarker{};


            if (!ReadExact(
                file,
                &fileMarker,
                sizeof(fileMarker)
            ))
            {
                delete[] markers;
                std::free(vertices);

                sceIoClose(file);

                return false;
            }


            PspMeshMarker& marker =
                markers[i];


            marker.type =
                static_cast<PspMeshMarkerType>(
                    fileMarker.type
                );


            std::memcpy(
                marker.name,
                fileMarker.name,
                sizeof(marker.name)
            );


            marker.name[
                sizeof(marker.name) - 1
            ] = '\0';


            marker.position =
            {
                fileMarker.px,
                fileMarker.py,
                fileMarker.pz
            };


            marker.forward =
            {
                fileMarker.fx,
                fileMarker.fy,
                fileMarker.fz
            };
        }
    }


    sceIoClose(file);


    m_vertices =
        vertices;

    m_markers =
        markers;

    m_vertexCount =
        static_cast<int>(
            header.vertexCount
        );

    m_markerCount =
        static_cast<int>(
            header.markerCount
        );

    m_boundingRadius =
        header.boundingRadius;


    /*
        A Geometry Engine közvetlenül fogja
        olvasni a vertex buffert.
    */
    sceKernelDcacheWritebackAll();


    return true;
}


void PspMeshResource::Unload()
{
    if (m_vertices != nullptr)
    {
        std::free(
            m_vertices
        );

        m_vertices =
            nullptr;
    }


    if (m_markers != nullptr)
    {
        delete[] m_markers;

        m_markers =
            nullptr;
    }


    m_vertexCount = 0;
    m_markerCount = 0;

    m_boundingRadius = 0.0f;
}


bool PspMeshResource::IsLoaded() const
{
    return
        m_vertices != nullptr &&
        m_vertexCount > 0;
}


void PspMeshResource::BindTo(
    Mesh& mesh
) const
{
    if (!IsLoaded())
    {
        return;
    }


    mesh.SetData(
        m_vertices,

        m_vertexCount,

        GU_TRIANGLES,

        GU_TEXTURE_32BITF |
        GU_NORMAL_32BITF |
        GU_VERTEX_32BITF |
        GU_TRANSFORM_3D
    );
}


float PspMeshResource::GetBoundingRadius() const
{
    return m_boundingRadius;
}


int PspMeshResource::GetMarkerCount() const
{
    return m_markerCount;
}


const PspMeshMarker*
PspMeshResource::GetMarker(
    int index
) const
{
    if (
        index < 0 ||
        index >= m_markerCount
    )
    {
        return nullptr;
    }


    return
        &m_markers[index];
}