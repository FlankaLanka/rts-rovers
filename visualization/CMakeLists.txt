cmake_minimum_required(VERSION 3.16)
project(TerrafirmaVisualization VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    # Try pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW QUIET glfw3)
    endif()
endif()

if(NOT glfw3_FOUND AND NOT GLFW_FOUND)
    message(FATAL_ERROR "GLFW3 not found. Please install it:\n  macOS: brew install glfw\n  Ubuntu: sudo apt-get install libglfw3-dev")
endif()

# Find GLM
find_package(glm QUIET)
if(NOT glm_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLM QUIET glm)
    endif()
endif()

# GLAD sources
set(GLAD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c
)

# ImGui sources
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Application sources
set(APP_SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/Timer.cpp
    src/network/UDPReceiver.cpp
    src/network/PacketParser.cpp
    src/data/RoverData.cpp
    src/data/PointCloud.cpp
    src/data/DataManager.cpp
    src/render/Shader.cpp
    src/render/Camera.cpp
    src/render/Renderer.cpp
    src/render/RoverRenderer.cpp
    src/render/PointCloudRenderer.cpp
    src/render/TerrainRenderer.cpp
    src/render/CircleRenderer.cpp
    src/terrain/TerrainRaycast.cpp
    src/terrain/TerrainOperation.cpp
    src/ui/UIManager.cpp
    src/ui/SciFiTheme.cpp
)

# Create executable
add_executable(terrafirma_viz
    ${APP_SOURCES}
    ${GLAD_SOURCES}
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(terrafirma_viz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external  # For tiny_obj_loader.h
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link libraries
target_link_libraries(terrafirma_viz PRIVATE
    OpenGL::GL
)

# Platform-specific linking
if(APPLE)
    target_link_libraries(terrafirma_viz PRIVATE
        glfw
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
else()
    target_link_libraries(terrafirma_viz PRIVATE
        glfw
        dl
        pthread
    )
endif()

# GLM include
if(glm_FOUND)
    target_link_libraries(terrafirma_viz PRIVATE glm::glm)
elseif(GLM_FOUND)
    target_include_directories(terrafirma_viz PRIVATE ${GLM_INCLUDE_DIRS})
else()
    # Try to find GLM headers directly
    find_path(GLM_INCLUDE_DIR glm/glm.hpp
        PATHS /usr/include /usr/local/include /opt/homebrew/include
    )
    if(GLM_INCLUDE_DIR)
        target_include_directories(terrafirma_viz PRIVATE ${GLM_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "GLM not found. Please install it:\n  macOS: brew install glm\n  Ubuntu: sudo apt-get install libglm-dev")
    endif()
endif()

# Copy shaders to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/render/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy assets folder to build directory (for model files)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Installation
install(TARGETS terrafirma_viz RUNTIME DESTINATION bin)

