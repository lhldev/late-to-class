cmake_minimum_required(VERSION 3.10)
project(LateToClass C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type: Debug or Release" FORCE)
endif()

message(STATUS "Building in ${CMAKE_BUILD_TYPE} mode")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

set(SANITIZE_FLAGS -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
set(WARNING_FLAGS -Wall -Wextra -Wpedantic)
set(RELEASE_FLAGS -O3 -DNDEBUG -march=native)
set(PROFILE_FLAG -O3 -p -g)
add_compile_options(${WARNING_FLAGS})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(${SANITIZE_FLAGS} -g)
    add_link_options(${SANITIZE_FLAGS})
elseif(CMAKE_BUILD_TYPE STREQUAL "Profile")
    add_compile_options(${PROFILE_FLAG})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(${RELEASE_FLAGS})
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E echo "Building in: ${CMAKE_BUILD_TYPE} mode"
    COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
)
