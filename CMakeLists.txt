# Set the minimum required version of CMake.
cmake_minimum_required(VERSION 3.25)

# Set compiler before project() so CMake uses it from the start
if(NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER clang++)
endif()

project(MyOpenGLProject CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(GLEW   REQUIRED)
find_package(glfw3  REQUIRED)
find_package(assimp REQUIRED)

# --- Sources ---

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER APP_SOURCES EXCLUDE REGEX ".*/vendor/.*")
list(FILTER APP_SOURCES EXCLUDE REGEX ".*/Platform/System/Input\\.cpp$")

set(VENDOR_SOURCES
    src/vendor/imgui/imgui.cpp
    src/vendor/imgui/imgui_draw.cpp
    src/vendor/imgui/imgui_tables.cpp
    src/vendor/imgui/imgui_widgets.cpp
    src/vendor/imgui/imgui_demo.cpp
    src/vendor/imgui/backends/imgui_impl_glfw.cpp
    src/vendor/imgui/backends/imgui_impl_opengl3.cpp
    src/vendor/stb_image/stb_image.cpp
)

add_executable(MyOpenGLApp ${APP_SOURCES} ${VENDOR_SOURCES})

# --- Include paths ---

target_include_directories(MyOpenGLApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/imgui
)

# --- Compiler flags ---

target_compile_options(MyOpenGLApp PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    $<$<CONFIG:Debug>:  -O0 -g -fsanitize=address,undefined>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)

target_compile_definitions(MyOpenGLApp PRIVATE
    $<$<CONFIG:Debug>:LSAN_SUPPRESSIONS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lsan.supp">
)

target_link_options(MyOpenGLApp PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

# --- Vendor: suppress warnings on code that isn't ours ---

set_source_files_properties(${VENDOR_SOURCES} PROPERTIES
    COMPILE_FLAGS "-w"
)

# --- Link ---

target_link_libraries(MyOpenGLApp PRIVATE
    OpenGL::GL
    assimp::assimp
    GLEW::glew
    glfw
)