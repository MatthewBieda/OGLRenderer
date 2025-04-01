cmake_minimum_required(VERSION 3.30)  # Ensure a stable CMake version
project(OGLRenderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make sure the runtime library is statically linked
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Global settings to reduce build size
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)  # Force static linking to avoid unused shared objects

# Assimp (Skip tests, tools)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)

# GLFW (Skip examples, tests, docs)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# ImGui (Avoid extra backend builds)
set(IMGUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(IMGUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Glad (No need for extra loaders)
set(GLAD_INSTALL OFF CACHE BOOL "" FORCE)

# FetchContent Module
include(FetchContent)

# ==========================
# Third-Party Dependencies
# ==========================

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.7
)
FetchContent_MakeAvailable(glfw)

# GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
FetchContent_MakeAvailable(glad)

# GLM (Header-Only, No Build Needed)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)  

# ImGui (Docking Branch)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# Assimp (Model Loading)
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.4.3
)
FetchContent_MakeAvailable(assimp)

# stb_image (Header-Only)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# ==========================
# Include Project Files
# ==========================
add_executable(OGLRenderer 
    src/main.cpp
    src/camera.cpp
    src/mesh.cpp
    src/model.cpp
    src/shader.cpp

    # ImGui core files
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp

    # ImGui platform & rendering backends
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ==========================
# Include Directories
# ==========================
target_include_directories(OGLRenderer PRIVATE include)

# Include directories for third-party dependencies
target_include_directories(OGLRenderer PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${stb_SOURCE_DIR}
)

# ==========================
# Link Libraries
# ==========================
target_link_libraries(OGLRenderer PRIVATE glfw glad glm assimp)

# Enable Multi-Core Compilation for Faster Builds
if (MSVC)
    target_compile_options(OGLRenderer PRIVATE /MP)
endif()

# Set Working Directory for Assets
target_compile_definitions(OGLRenderer PRIVATE ASSETS_PATH="${CMAKE_SOURCE_DIR}/assets/")

# ==========================
# Final Message
# ==========================
message(STATUS "OGLRenderer setup complete!")
