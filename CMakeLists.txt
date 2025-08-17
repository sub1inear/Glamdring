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
# enable AVX-512
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX512 /Ox)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -mavx512f -mavx512dq -mavx512bw -mavx512vl -mbmi -mbmi2 -O3)
endif()