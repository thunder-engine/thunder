cmake_minimum_required(VERSION 3.10)

project(iostools)

file(GLOB ${PROJECT_NAME}_srcFiles
    "*.cpp"
    "converter/*.cpp"
    "*.qrc"
    "*.h"
    "converter/*.h"
)

set(${PROJECT_NAME}_incPaths
    "../../../"
    "../../../engine/includes"
    "../../../engine/includes/resources"
    "../../../engine/includes/editor"
    "../../../thirdparty/next/inc"
    "../../../thirdparty/next/inc/math"
    "../../../thirdparty/next/inc/core"
)

if (desktop AND APPLE)
    add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_srcFiles})

    target_link_libraries(${PROJECT_NAME} PRIVATE
        next-editor
        engine-editor
        Qt5::Core
        Qt5::Gui
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        SHARED_DEFINE
    )

    target_include_directories(iostools PRIVATE ${${PROJECT_NAME}_incPaths})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}
            DESTINATION "${PLUGINS_PATH}"
    )

endif ()
