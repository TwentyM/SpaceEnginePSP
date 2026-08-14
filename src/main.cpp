#include <pspkernel.h>

#include "Engine/Core/Application.h"

PSP_MODULE_INFO("SpaceEnginePrototype", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int main()
{
    Application application;
    application.Run();

    return 0;
}