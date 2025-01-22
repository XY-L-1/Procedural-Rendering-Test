cmake_minimum_required(VERSION 3.10)
project(ProceduralTerrain)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include)
include_directories(include/GLFW)
include_directories(include/GLAD)
include_directories(include/GLM)
include_directories(include/FastNoiseLite)

# Link directories
link_directories(lib/GLFW)

# Add executable
add_executable(ProceduralTerrain src/main.cpp src/glad.c)

# Copy shaders and models to the build directory
file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
message("Copying shaders to: ${CMAKE_BINARY_DIR}/shaders")

file(COPY obj DESTINATION ${CMAKE_BINARY_DIR})
message("Copying obj to: ${CMAKE_BINARY_DIR}/obj")


# Link libraries
target_link_libraries(ProceduralTerrain glfw3 opengl32 gdi32)