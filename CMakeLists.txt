# step 1: basic setting (just use the default configuration)
cmake_minimum_required(VERSION 3.25)
project(GLSLPathTracer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# step 2: set custom variables
set(SDL2_LIBDIR ${CMAKE_SOURCE_DIR}/thirdparty/SDL2/bin)
set(OIDN_LIBDIR ${CMAKE_SOURCE_DIR}/thirdparty/oidn/lib)
set(SDL2_LIBRARIES "SDL2" "SDL2main")
set(OIDN_LIBRARIES "OpenImageDenoise")
set(EXE_NAME "GLSLPathTracer")

# step 3: collect files as custom variables
# file(GLOB_RECURSE XXX ...): find all matched files and stored in XXX
file(GLOB_RECURSE SHADERS
        ${CMAKE_SOURCE_DIR}/src/shaders/*.glsl
)
file(GLOB_RECURSE SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/*.h
        ${CMAKE_SOURCE_DIR}/src/*.cpp
)
file(GLOB_RECURSE EXT_FILES
        ${CMAKE_SOURCE_DIR}/thirdparty/stb/*.h
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.h
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.cpp
        ${CMAKE_SOURCE_DIR}/thirdparty/imguizmo/*.h
        ${CMAKE_SOURCE_DIR}/thirdparty/imguizmo/*.cpp
        ${CMAKE_SOURCE_DIR}/thirdparty/RadeonRays/*.h
        ${CMAKE_SOURCE_DIR}/thirdparty/RadeonRays/*.cpp
        ${CMAKE_SOURCE_DIR}/thirdparty/oidn/*.h
        ${CMAKE_SOURCE_DIR}/thirdparty/oidn/*.hpp
        ${CMAKE_SOURCE_DIR}/thirdparty/tinydir/*.h
        ${CMAKE_SOURCE_DIR}/thirdparty/tinyobjloader/*.h
        ${CMAKE_SOURCE_DIR}/thirdparty/tinygltf/*.h
        ${CMAKE_SOURCE_DIR}/thirdparty/gl3w/GL/gl3w.c
        ${CMAKE_SOURCE_DIR}/thirdparty/blender
)

# step 4: include directories with only *.h
include_directories(
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/math
        ${CMAKE_SOURCE_DIR}/src/loaders
        ${CMAKE_SOURCE_DIR}/thirdparty/stb
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui
        ${CMAKE_SOURCE_DIR}/thirdparty/imguizmo
        ${CMAKE_SOURCE_DIR}/thirdparty/tinydir
        ${CMAKE_SOURCE_DIR}/thirdparty/oidn/include
        ${CMAKE_SOURCE_DIR}/thirdparty/RadeonRays
        ${CMAKE_SOURCE_DIR}/thirdparty/tinyobjloader
        ${CMAKE_SOURCE_DIR}/thirdparty/tinygltf
        ${CMAKE_SOURCE_DIR}/thirdparty/gl3w
        ${CMAKE_SOURCE_DIR}/thirdparty/blender
)

# step 5: link directories with *.dll
link_directories(
        ${SDL2_LIBDIR}
        ${OIDN_LIBDIR}
        ${GLFW_LIBDIR}
)

# step 6: especially set for OpenGL
find_package(OpenGL)

# step 7: add executable
set(SRCS ${SRC_FILES} ${EXT_FILES} ${SHADERS})
add_executable(${EXE_NAME} ${SRCS})

# step 8: link libraries with /lib or dll (and add mingw32)
target_link_libraries(${EXE_NAME} mingw32 ${SDL2_LIBRARIES} ${OIDN_LIBRARIES} ${OPENGL_LIBRARIES})

