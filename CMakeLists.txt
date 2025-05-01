cmake_minimum_required(VERSION 3.10)
project(ProjetOpenGL)

set(CMAKE_CXX_STANDARD 17)

# Inclure les headers
include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/libs)

# Ajouter les sources
file(GLOB SRC_FILES src/*.cpp)
add_executable(ProjetOpenGL ${SRC_FILES})

# Lier les bibliothèques
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)

target_link_libraries(ProjetOpenGL
    OpenGL32
    glfw3
    glew32
)
