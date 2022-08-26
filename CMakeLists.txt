project(dem-editor)
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE INTERNAL "" FORCE)

add_executable(dem-editor
        ${PROJECT_SOURCE_DIR}/src/main.cpp
)
find_package(gaunlet REQUIRED)
target_link_libraries(dem-editor PUBLIC gaunlet::gaunlet)
