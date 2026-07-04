cmake_minimum_required(VERSION 3.16)
project(BunkerWorldEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Здесь строго без версии 2.5 и компоненты с Большой буквы!
find_package(SFML REQUIRED COMPONENTS Graphics Window System)
find_package(ImGui-SFML REQUIRED)

set(SOURCES
    src/main.cpp
    src/PlacementSystem.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

# Меняем sfml-graphics / SFML::graphics на правильные SFML::Graphics с большой буквы
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGui-SFML::ImGui-SFML
)



