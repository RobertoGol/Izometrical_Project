if(NOT DEFINED APP_PATH)
    message(FATAL_ERROR "APP_PATH is required")
endif()

if(NOT EXISTS "${APP_PATH}")
    message(FATAL_ERROR "Executable is missing: ${APP_PATH}")
endif()

get_filename_component(APP_DIR "${APP_PATH}" DIRECTORY)

set(REQUIRED_ASSETS
    "${APP_DIR}/assets/shaders/base.vert"
    "${APP_DIR}/assets/shaders/base.frag"
)

foreach(asset IN LISTS REQUIRED_ASSETS)
    if(NOT EXISTS "${asset}")
        message(FATAL_ERROR "Required runtime asset is missing: ${asset}")
    endif()
endforeach()

message(STATUS "Smoke test passed for ${APP_PATH}")
