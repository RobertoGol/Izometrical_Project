cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolISO VERSION 1.0 LANGUAGES CXX)

# ==============================================================================
# C++17 (нужен для std::filesystem, std::clamp, inline constexpr)
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================================================================
# Найти SFML (2.5+)
# Установка:
#   vcpkg: vcpkg install sfml
#   apt:   sudo apt install libsfml-dev
#   brew:  brew install sfml
# ==============================================================================
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# ==============================================================================
# Исходники
# Большинство legacy-систем пока header-only (в include/), но runtime/render
# постепенно выносятся из main.cpp в нормальные компилируемые модули.
# ==============================================================================
set(SOURCES
    src/main.cpp
    src/engine/GameApplication.cpp
    src/render/GameRenderer.cpp
    src/entities/PlayerController.cpp
)

# ==============================================================================
# Исполняемый файл
# ==============================================================================
add_executable(${PROJECT_NAME} ${SOURCES})

# ==============================================================================
# Include-директории
# ==============================================================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ==============================================================================
# Линковка SFML
# ==============================================================================
target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# ==============================================================================
# Для std::filesystem на некоторых компиляторах (GCC < 9)
# ==============================================================================
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

# ==============================================================================
# Копируем assets в build-директорию
# ==============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Copying assets to build directory..."
    )
endif()

# ==============================================================================
# Предупреждения компилятора
# ==============================================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /wd4100  # неиспользованные параметры
        /wd4244  # double -> float
        /wd4267  # size_t -> int
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
    )
endif()
