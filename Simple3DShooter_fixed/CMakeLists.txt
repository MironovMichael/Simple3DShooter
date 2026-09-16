cmake_minimum_required(VERSION 3.16)
project(Simple3DShooter VERSION 1.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# V25 build quality: predictable release defaults and stronger diagnostics.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

set(SOURCES
    src/main.cpp
    src/Item.cpp
    src/ItemManager.cpp
    src/Object.cpp
    src/Player.cpp
    src/Story.cpp
    src/Weapon.cpp
    src/World.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-ffunction-sections;-fdata-sections>
    $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
)

# Keep the executable lean when the linker supports section garbage collection.
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wl,--gc-sections>
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL)


# V25 quality-pass notes are documented in V25_FILE_AUDIT.txt.
