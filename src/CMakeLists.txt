add_executable(terramorph
        terramorph/main.cpp

        # graphics
        terramorph/core/graphics/procedural-plane/ProceduralPlane.cpp
        terramorph/core/graphics/render-pipeline/TerrainRenderPipeline.cpp
        terramorph/core/graphics/terrain-components/TerrainComponent.cpp
)

# Libraries
find_package(gaunlet REQUIRED)
target_link_libraries(terramorph PUBLIC gaunlet::gaunlet)

# Headers
target_include_directories(terramorph
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Precompiled headers
target_precompile_headers(terramorph
        PRIVATE terramorph/pch.h
)

# Compiler definitions
target_compile_definitions(terramorph
        PUBLIC
            WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}/terramorph"
            ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/terramorph/assets"
)
