#pragma once

#include <cstdint>


class PspTextureResource
{
public:
    PspTextureResource();

    ~PspTextureResource();

    PspTextureResource(
        const PspTextureResource&
    ) = delete;

    PspTextureResource& operator=(
        const PspTextureResource&
    ) = delete;


    bool Load(
        const char* path
    );

    void Unload();


    bool IsLoaded() const;


    void Bind() const;

    void Unbind() const;


    int GetWidth() const;

    int GetHeight() const;


private:
    void* m_data;

    int m_width;
    int m_height;
    int m_format;
};