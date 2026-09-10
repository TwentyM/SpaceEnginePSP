#include "PspTextureResource.h"

#include <pspiofilemgr.h>
#include <pspkernel.h>
#include <pspgu.h>

#include <malloc.h>

#include <cstdlib>
#include <cstring>


namespace
{
    constexpr std::uint32_t
        ExpectedVersion = 1;


#pragma pack(push, 1)

    struct FileHeader
    {
        char magic[8];

        std::uint32_t version;

        std::uint32_t width;
        std::uint32_t height;

        std::uint32_t format;

        std::uint32_t dataSize;
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
            'T',
            'X',
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


    bool IsPowerOfTwo(
        std::uint32_t value
    )
    {
        return
            value != 0 &&
            (
                value &
                (value - 1)
            ) == 0;
    }
}


PspTextureResource::PspTextureResource()
    : m_data(nullptr),
      m_width(0),
      m_height(0),
      m_format(0)
{
}


PspTextureResource::~PspTextureResource()
{
    Unload();
}


bool PspTextureResource::Load(
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
        header.version !=
            ExpectedVersion ||
        !IsPowerOfTwo(
            header.width
        ) ||
        !IsPowerOfTwo(
            header.height
        ) ||
        header.width > 512 ||
        header.height > 512 ||
        header.format !=
            GU_PSM_5650
    )
    {
        sceIoClose(file);

        return false;
    }


    const std::uint32_t
        expectedSize =
            header.width *
            header.height *
            2;


    if (
        header.dataSize !=
        expectedSize
    )
    {
        sceIoClose(file);

        return false;
    }


    void* data =
        memalign(
            16,
            header.dataSize
        );


    if (data == nullptr)
    {
        sceIoClose(file);

        return false;
    }


    if (!ReadExact(
        file,
        data,
        header.dataSize
    ))
    {
        std::free(data);

        sceIoClose(file);

        return false;
    }


    sceIoClose(file);


    m_data = data;

    m_width =
        static_cast<int>(
            header.width
        );

    m_height =
        static_cast<int>(
            header.height
        );

    m_format =
        static_cast<int>(
            header.format
        );


    sceKernelDcacheWritebackInvalidateAll();


    return true;
}


void PspTextureResource::Unload()
{
    if (m_data != nullptr)
    {
        std::free(
            m_data
        );

        m_data = nullptr;
    }


    m_width = 0;
    m_height = 0;
    m_format = 0;
}


bool PspTextureResource::IsLoaded() const
{
    return
        m_data != nullptr &&
        m_width > 0 &&
        m_height > 0;
}


void PspTextureResource::Bind() const
{
    if (!IsLoaded())
    {
        sceGuDisable(
            GU_TEXTURE_2D
        );

        return;
    }


    sceGuEnable(
        GU_TEXTURE_2D
    );


    /*
        Első verzió:
        linear main-RAM texture.

        Később:
        swizzled + lehetőleg VRAM.
    */
    sceGuTexMode(
        m_format,
        0,
        0,
        GU_FALSE
    );


    sceGuTexImage(
        0,
        m_width,
        m_height,
        m_width,
        m_data
    );


    /*
        Texture * lighting.

        Így a Blender textúra színeit
        a directional light továbbra is
        világosítja/sötétíti.
    */
    sceGuTexFunc(
        GU_TFX_MODULATE,
        GU_TCC_RGB
    );


    sceGuTexMapMode(
        GU_TEXTURE_COORDS,
        0,
        0
    );


    sceGuTexScale(
        1.0f,
        1.0f
    );


    sceGuTexOffset(
        0.0f,
        0.0f
    );


    sceGuTexFilter(
        GU_LINEAR,
        GU_LINEAR
    );


    sceGuTexWrap(
        GU_REPEAT,
        GU_REPEAT
    );
}


void PspTextureResource::Unbind() const
{
    sceGuDisable(
        GU_TEXTURE_2D
    );
}


int PspTextureResource::GetWidth() const
{
    return m_width;
}


int PspTextureResource::GetHeight() const
{
    return m_height;
}