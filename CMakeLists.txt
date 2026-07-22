# Set the minimum required version of CMake.
cmake_minimum_required(VERSION 3.25)

# Set compiler before project() so CMake uses it from the start
if(NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER clang++)
endif()

project(Sculpt CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(GLEW   REQUIRED)
find_package(glfw3  REQUIRED)
find_package(assimp REQUIRED)

# vcpkg static exports GLEW::GLEW; system packages export ${GLEW_TARGET}
if(TARGET ${GLEW_TARGET})
    set(GLEW_TARGET ${GLEW_TARGET})
else()
    set(GLEW_TARGET GLEW::GLEW)
endif()

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

add_executable(Sculpt ${APP_SOURCES} ${VENDOR_SOURCES})

# --- Include paths ---

target_include_directories(Sculpt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/imgui
)

# --- Compiler flags ---

target_compile_options(Sculpt PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wall -Wextra -Wpedantic -Wno-unused-parameter>
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /wd4100 /utf-8>
    $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:
        -O0 -g -fsanitize=address,undefined>
    $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:
        -O2 -DNDEBUG>
)

target_compile_definitions(Sculpt PRIVATE
    FMT_HEADER_ONLY
    $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:
        LSAN_SUPPRESSIONS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lsan.supp">
)

target_link_options(Sculpt PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:
        -fsanitize=address,undefined>
)

# --- Vendor: suppress warnings on code that isn't ours ---

set_source_files_properties(${VENDOR_SOURCES} PROPERTIES
    COMPILE_OPTIONS "$<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>"
)

# --- Link ---

target_link_libraries(Sculpt PRIVATE
    OpenGL::GL
    assimp::assimp
    ${GLEW_TARGET}
    glfw
)

# --- Math tests ---

find_package(GTest)
if(GTest_FOUND)
    add_executable(MathTest
        tests/math_test.cpp
        src/Platform/CoreUtils/Math/maths.cpp
    )
    target_include_directories(MathTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_options(MathTest PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_features(MathTest PRIVATE cxx_std_23)
    target_link_libraries(MathTest PRIVATE ${GLEW_TARGET} GTest::GTest GTest::Main)
endif()