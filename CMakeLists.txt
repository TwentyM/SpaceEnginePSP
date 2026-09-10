cmake_minimum_required(VERSION 3.11)

project(SpaceEnginePrototype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}

    src/main.cpp

    src/Engine/Core/Application.cpp

    src/Engine/Graphics/Renderer.cpp
    src/Engine/Graphics/Camera.cpp
    src/Engine/Graphics/Mesh.cpp
    src/Engine/Graphics/DirectionalLight.cpp
    src/Engine/Graphics/Starfield.cpp
    src/Engine/Graphics/EnginePlumeRenderer.cpp

    src/Engine/Resources/PspMeshResource.cpp
    src/Engine/Resources/PspTextureResource.cpp

    src/Engine/Input/Input.cpp
    
    src/Game/Ships/PlayerShip.cpp
    src/Game/Combat/ProjectileSystem.cpp
    src/Game/Game.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE

    pspgum
    pspgu

    pspctrl

    pspge
    pspdisplay

    m
)

create_pbp_file(
    TARGET ${PROJECT_NAME}

    ICON_PATH NULL
    BACKGROUND_PATH NULL
    PREVIEW_PATH NULL

    TITLE "SpaceEngine Prototype"

    VERSION 01.00
)