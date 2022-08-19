cmake_minimum_required(VERSION 3.1)
project(Chip8)

set(CMAKE_C_STANDARD 11)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SOURCES
    "${SRC_DIR}/main.c"
    "${SRC_DIR}/chip8.c"
    "${SRC_DIR}/renderer.c"
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")

set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

FILE(COPY c8games/debug.ch8 DESTINATION ${CMAKE_BINARY_DIR})

# GLFW
set(GLFW_DIR "${LIB_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory(${GLFW_DIR})
target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

# GLAD
set(GLAD_DIR "${LIB_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/gl.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad")

# STB_IMAGE
set(STB_IMG_DIR "${LIB_DIR}/stb_image")
add_library("stb_image" "${STB_IMG_DIR}/stb_image.c")
target_include_directories("stb_image" PRIVATE "${STB_IMG_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${LIB_DIR}/stb_image")
target_link_libraries(${PROJECT_NAME} "stb_image")
