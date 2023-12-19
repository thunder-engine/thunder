cmake_minimum_required(VERSION 3.10)

project(pipelinetools)

file(GLOB ${PROJECT_NAME}_srcFiles
    "*.cpp"
    "converter/*.cpp"
    "editor/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "../../../engine/includes"
    "../../../engine/includes/resources"
    "../../../engine/includes/components"
    "../../../thirdparty/next/inc"
    "../../../thirdparty/next/inc/math"
    "../../../thirdparty/next/inc/core"
    "../../../modules/editor/grapheditor"
)

file(GLOB ${PROJECT_NAME}_ui
    "editor/*.ui"
)

file(GLOB MOC_HEADERS
    "*.h"
    "converter/*.h"
    "editor/*.h"
)

set( RESOURCES
    pipeline.qrc
)

QT5_ADD_RESOURCES(RES_SOURCES ${RESOURCES})
QT5_WRAP_UI(UI_HEADERS ${${PROJECT_NAME}_ui})
QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

add_definitions (
    -DCOMPANY_NAME="${COMPANY_NAME}"
    -DPRODUCT_NAME="${PRODUCT_NAME}"
    -DEDITOR_NAME="${EDITOR_NAME}"
    -DBUILDER_NAME="${BUILDER_NAME}"
    -DSDK_VERSION="${SDK_VERSION}"
    -DCOPYRIGHT_YEAR=${COPYRIGHT_YEAR}
    -DCOPYRIGHT_AUTHOR="${COPYRIGHT_AUTHOR}"
    -DREVISION="${REVISION}"
    -DLEGAL="${LEGAL}"
)

if (desktop)
    add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_srcFiles} ${UI_HEADERS} ${MOC_SRCS} ${RES_SOURCES})

    target_link_libraries(${PROJECT_NAME} PRIVATE
        next-editor
        engine-editor
        graph-editor
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Xml
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
endif()

if (desktop AND UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_RPATH "$ORIGIN/../../lib"
    )
endif()

if (desktop AND APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_NAME_DIR "@executable_path"
    )
endif()
