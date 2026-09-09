#include <pspkernel.h>
#include <pspiofilemgr.h>

#include <cstring>

#include "Engine/Core/Application.h"


PSP_MODULE_INFO(
    "SpaceEnginePrototype",
    0,
    1,
    0
);

PSP_MAIN_THREAD_ATTR(
    THREAD_ATTR_USER |
    THREAD_ATTR_VFPU
);


namespace
{
    void SetWorkingDirectoryFromBootPath(
        int argc,
        char* argv[]
    )
    {
        if (
            argc <= 0 ||
            argv == nullptr ||
            argv[0] == nullptr
        )
        {
            return;
        }


        char path[512];


        std::strncpy(
            path,
            argv[0],
            sizeof(path) - 1
        );


        path[
            sizeof(path) - 1
        ] = '\0';


        char* separator =
            std::strrchr(
                path,
                '/'
            );


        char* backslash =
            std::strrchr(
                path,
                '\\'
            );


        if (
            backslash != nullptr &&
            (
                separator == nullptr ||
                backslash > separator
            )
        )
        {
            separator =
                backslash;
        }


        if (separator == nullptr)
        {
            return;
        }


        *separator = '\0';


        sceIoChdir(
            path
        );
    }
}


int main(
    int argc,
    char* argv[]
)
{
    SetWorkingDirectoryFromBootPath(
        argc,
        argv
    );


    Application application;

    application.Run();


    return 0;
}