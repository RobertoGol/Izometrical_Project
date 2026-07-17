cmake_minimum_required(VERSION 3.15)
project(MapEditor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "Repository root")
set(SFML_ROOT "${PROJECT_ROOT}/source/SFML-3.1.0" CACHE PATH "Bundled SFML root")
set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML" CACHE PATH "SFML CMake package directory")
set(IMGUI_DIR "${PROJECT_ROOT}/source/ImGui")

find_package(SFML 3.1 CONFIG REQUIRED COMPONENTS Graphics Window System)
find_package(OpenGL REQUIRED)

add_library(map_editor_imgui_sfml STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui-SFML.cpp"
)

target_include_directories(map_editor_imgui_sfml PUBLIC
    "${IMGUI_DIR}"
)

target_link_libraries(map_editor_imgui_sfml PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
    OpenGL::GL
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/MapEditorConsole.cpp
    src/MapEditorInput.cpp
    src/MapEditorPanels.cpp
    src/MapEditorPlacement.cpp
    src/MapEditorView.cpp
    src/PlacementSystem.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    map_editor_imgui_sfml
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(EXISTS "${SFML_ROOT}/bin")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SFML_ROOT}/bin"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying SFML runtime DLLs to build directory..."
    )
endif()
