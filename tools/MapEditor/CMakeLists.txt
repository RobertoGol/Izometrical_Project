cmake_minimum_required(VERSION 3.15)
project(MapEditor CXX)

# ПРИНУДИТЕЛЬНО подключаем vcpkg (Добавьте эту строку!)
set(CMAKE_TOOLCHAIN_FILE "F:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)

# Включаем современный стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Экспортируем команды для IntelliSense/Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Поиск SFML 3 и ImGui
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp 
    src/PlacementSystem.cpp
)

# Передаем пути и библиотеки в проект
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGui-SFML::ImGui-SFML
)
