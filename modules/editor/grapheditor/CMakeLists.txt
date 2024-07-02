cmake_minimum_required(VERSION 3.10)

project(graph)

file(GLOB ${PROJECT_NAME}_srcFiles
    "*.cpp"
    "editor/graph/*.cpp"
    "editor/graph/*.h"
    "editor/graph/graphwidgets/*.cpp"
    "*.h"
)

set(${PROJECT_NAME}_incPaths
    "./"
    "../../../engine/includes"
    "../../../engine/includes/resources"
    "../../../engine/includes/components"
    "../../../engine/includes/editor"
    "../../../thirdparty/next/inc"
    "../../../thirdparty/next/inc/math"
    "../../../thirdparty/next/inc/core"
    "../../../modules/uikit/includes"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

file(GLOB MOC_HEADERS
    "editor/graph/*.h"
    "editor/graph/graphwidgets/*.h"
)

set(RESOURCES
    grapheditor.qrc
)

QT5_ADD_RESOURCES(RES_SOURCES ${RESOURCES})
QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

if (desktop)
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles} ${MOC_SRCS} ${RES_SOURCES})

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        next-editor
        engine-editor
        uikit-editor
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Xml
    )

    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
        NODEGRAPH_LIBRARY
    )

    if(UNIX AND NOT APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
        )

        # Solve build error using Clang on BSDs
        if (NOT LINUX)
            target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
        endif()
    endif()

    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../../${LIB_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}-editor
            LIBRARY DESTINATION ${LIB_PATH}
            ARCHIVE DESTINATION ${LIB_PATH}
            RUNTIME DESTINATION ${LIB_PATH}
    )

    install(DIRECTORY editor/graph
            DESTINATION "${INC_PATH}/editor"
            FILES_MATCHING PATTERN "*.h"
            PATTERN "graphwidgets" EXCLUDE
    )

endif()
