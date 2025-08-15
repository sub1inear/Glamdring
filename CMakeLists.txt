cmake_minimum_required(VERSION 3.16)

project(Glamdring)

set(CMAKE_CXX_STANDARD 20)

# add all source files
file(GLOB SOURCES "src/*.h" "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# enable AVX-512
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX512)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -mavx512f -mavx512dq -mavx512bw -mavx512vl -mbmi -mbmi2)
endif()
