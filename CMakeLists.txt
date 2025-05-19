cmake_minimum_required(VERSION 3.10)
project(ProjetOpenGL)
set(CMAKE_CXX_STANDARD 17)

# ---- includes de dossier “include/” et “libs/” si besoin ----
include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/libs)

# ---- Ton exécutable principal ----
file(GLOB SRC_FILES src/*.cpp)
add_executable(ProjetOpenGL ${SRC_FILES})

# ---- OpenGL (WIN32) et GLM (header-only) ----
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)

# ---- GLFW et GLEW (MSYS2) ----
# on suppose qu'ils sont installés dans /mingw64 via pacman
# et qu’on linke directement les .a qui s’appellent libglfw3.a et libglew32.a  
find_library(GLFW3_LIB   glfw3   PATHS ${CMAKE_SYSTEM_PREFIX_PATH}/lib)
find_library(GLEW32_LIB  glew32  PATHS ${CMAKE_SYSTEM_PREFIX_PATH}/lib)

if(NOT GLFW3_LIB)
  message(FATAL_ERROR "Impossible de trouver libglfw3.a")
endif()
if(NOT GLEW32_LIB)
  message(FATAL_ERROR "Impossible de trouver libglew32.a")
endif()

# ---- ImGui (static) ----
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/extern/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    #${IMGUI_DIR}/imgui_demo.cpp  # optionnel
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ---- Linking final ----
target_link_libraries(ProjetOpenGL
    PRIVATE
        imgui             # ImGui
        OpenGL::GL        # opengl32.lib
        ${GLFW3_LIB}      # libglfw3.a
        ${GLEW32_LIB}     # libglew32.a
        # GLM n'a pas besoin de .lib
)
