cmake_minimum_required(VERSION 3.16)
project(Simple3DShooter VERSION 1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

set(SOURCES
    src/main.cpp
    src/Enemy.cpp
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
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL)
