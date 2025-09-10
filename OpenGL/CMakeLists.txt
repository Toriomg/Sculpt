# Set the minimum required version of CMake.
cmake_minimum_required(VERSION 3.15)

# Set the project name and language.
project(MyOpenGLProject CXX)

# Use the C++17 standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
# --- Assimp Integration ---
# Since you have the assimp source code in 'src/vendor/assimp',
# we can tell CMake to build it as part of our project.
# This is a very clean and common way to handle vendored libraries.

# Disable building assimp's tools/tests to speed up the build.
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable Assimp Tests")
set(ASSIMP_INSTALL OFF CACHE BOOL "Disable Assimp Install")

# Add the assimp subdirectory. CMake will find its CMakeLists.txt
# and create a library target named 'assimp'.
add_subdirectory(src/vendor/assimp)

# --- Your Application ---

# Create your executable.
# You will need to list all your .cpp files here.
# For now, let's assume you have a main.cpp inside the 'src' folder.
add_executable(MyOpenGLApp src/main.cpp) # <--- Add your other .cpp files here!

# Tell your application where to find its own header files.
# This adds the entire 'src' directory to the include path.
target_include_directories(MyOpenGLApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link your application against the assimp library.
# CMake automatically handles including assimp's headers and linking the library
# because we used add_subdirectory().
target_link_libraries(MyOpenGLApp PRIVATE
    assimp
    GLEW::glew
    glfw3
)