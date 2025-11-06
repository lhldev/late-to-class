cmake_minimum_required(VERSION 3.10)
project(LateToClass C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type: Debug or Release" FORCE)
endif()

message(STATUS "Building in ${CMAKE_BUILD_TYPE} mode")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

set(FETCHCONTENT_QUIET FALSE CACHE BOOL "Disable quiet mode for FetchContent")
find_package(raylib ${RAYLIB_VERSION} QUIET)
if (NOT raylib_FOUND)
    message(STATUS "Downloading and building Raylib. This may take a few minutes on the first run...")
    include(FetchContent)
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        TIMEOUT 30
    )
    FetchContent_MakeAvailable(raylib)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include_directories(${CMAKE_SOURCE_DIR}/src)

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

target_link_libraries(${PROJECT_NAME} raylib)
if (UNIX)
    target_link_libraries(${PROJECT_NAME} m pthread dl rt X11 GL Xrandr Xinerama Xcursor Xxf86vm Xi)
elseif (WIN32)
    target_link_libraries(${PROJECT_NAME} winmm gdi32 opengl32)
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E echo "Building in: ${CMAKE_BUILD_TYPE} mode"
    COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
)
