cmake_minimum_required(VERSION 3.10)
set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic" CACHE STRING "" FORCE)
project(OpenGL_RayTracer)

set(CMAKE_CXX_STANDARD 17)

add_executable(OpenGL_RayTracer
        Camera.h
        main.cpp
        ObjectTexture.h
        RT_Screen.h
        Shader.h
        Tool.h
        Structure.hpp
        Model.hpp
)


find_package(glad CONFIG REQUIRED)
target_link_libraries(OpenGL_RayTracer PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(OpenGL_RayTracer PRIVATE glfw)
