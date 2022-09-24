project(dem-editor)
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE INTERNAL "" FORCE)

add_executable(dem-editor
        dem-editor/main.cpp

        # graphics
        dem-editor/graphics/components/procedural-plane/ProceduralPlane.cpp
        dem-editor/graphics/render-pipeline/TerrainRenderPipeline.cpp
)

# Libraries
find_package(gaunlet REQUIRED)
target_link_libraries(dem-editor PUBLIC gaunlet::gaunlet)

# Headers
target_include_directories(dem-editor
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Precompiled headers
target_precompile_headers(dem-editor
        PRIVATE dem-editor/pch.h
)

# Compiler definitions
target_compile_definitions(dem-editor
        PUBLIC
            WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}/dem-editor"
            ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/dem-editor/assets"
)
