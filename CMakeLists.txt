cmake_minimum_required(VERSION 3.10)
project(PortalGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# External libraries
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

# GLAD
add_library(glad external/glad/src/gl.c)
target_include_directories(glad PUBLIC external/glad/include)

# STB
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE external/stb)

# ImGui (Optional, but good to have setup if needed later, though not strictly requested for the core task yet)
# For now, I will skip ImGui to keep it simple as requested, but the folder is there.

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
# set(SOURCES
#     src/main.cpp
#     src/Application.cpp
#     src/Application.h
#     src/Camera.cpp
#     src/Camera.h
#     src/Mesh.cpp
#     src/Mesh.h
#     src/Model.cpp
#     src/Model.h
#     src/Shader.cpp
#     src/Shader.h
#     src/GameObject.h
#     src/Skybox.cpp
#     src/Skybox.h
#     src/Texture.cpp
#     src/Texture.h
#     src/stb_impl.cpp
# )

add_executable(PortalGame ${SOURCES})

target_include_directories(PortalGame PUBLIC 
    src 
    external/stb
)

target_link_libraries(PortalGame PUBLIC 
    glfw 
    glad 
    glm
    stb
)

if(APPLE)
    target_link_libraries(PortalGame PUBLIC "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()
