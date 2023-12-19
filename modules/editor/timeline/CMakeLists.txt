cmake_minimum_required(VERSION 3.10)

project(timeline)

file(GLOB ${PROJECT_NAME}_srcFiles
    "*.cpp"
    "editor/*.cpp"
    "editor/ui/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "editor"
    "../../../engine/includes"
    "../../../engine/includes/resources"
    "../../../engine/includes/editor"
    "../../../develop/managers/assetmanager"
    "../../../thirdparty/next/inc"
    "../../../thirdparty/next/inc/math"
    "../../../thirdparty/next/inc/core"
    "../../../thirdparty/next/inc/anim"
    "../../../thirdparty/syntaxhighlighting/src"
)

file(GLOB ${PROJECT_NAME}_ui
    "editor/*.ui"
)

file(GLOB MOC_HEADERS
    "*.h"
    "editor/*.h"
    "editor/ui/*.h"
)

set( RESOURCES
    timeline.qrc
)

QT5_ADD_RESOURCES(RES_SOURCES ${RESOURCES})
QT5_WRAP_UI(UI_HEADERS ${${PROJECT_NAME}_ui})
QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

add_definitions (
    -DCOMPANY_NAME="${COMPANY_NAME}"
    -DEDITOR_NAME="${EDITOR_NAME}"
)

if (desktop)
    add_library(${PROJECT_NAME} SHARED
        "../../../worldeditor/res/WorldEditor.qrc"
        ${${PROJECT_NAME}_srcFiles}
        ${UI_HEADERS}
        ${MOC_SRCS}
        ${RES_SOURCES}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        next-editor
        engine-editor
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        SHARED_DEFINE
    )

    target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../../${PLUGINS_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../../${PLUGINS_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}
            DESTINATION "${PLUGINS_PATH}"
    )

    if (desktop AND APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            INSTALL_NAME_DIR "@executable_path"
        )
    endif()

endif()
