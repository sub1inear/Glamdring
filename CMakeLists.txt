cmake_minimum_required(VERSION 3.16)

project(Glamdring)

set(CMAKE_CXX_STANDARD 20)

# add all source files
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# set working directory for VS debugger next to executable (initially in build directoyr)
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# copy data/Titans.bin
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/data/Titans.bin
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/Titans.bin
)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX512 /MP)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -mavx512f -mavx512dq -mavx512bw -mavx512vl -mbmi -mbmi2)
endif()

if (CMAKE_CONFIGURATION_TYPES)
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/Ox>)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
    endif()
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /Ox)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()
endif()
