cmake_minimum_required(VERSION 3.10)

project(editor)

set(${PROJECT_NAME}_incPaths
    "includes"
    "includes/editor"
    "../engine/includes"
    "../engine/includes/components"
    "../engine/includes/resources"
    "../thirdparty/next/inc"
    "../thirdparty/next/inc/math"
    "../thirdparty/next/inc/core"
    "../thirdparty/next/inc/anim"
    "../thirdparty/next/inc/os"
    "../thirdparty/assimp/include"
    "../thirdparty/pugixml/src"
)

add_definitions (
    -DCOMPANY_NAME="${COMPANY_NAME}"
    -DEDITOR_NAME="${EDITOR_NAME}"
    -DSDK_VERSION="${SDK_VERSION}"
)

if(desktop)
    # This path is only needed on the BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        set(${PROJECT_NAME}_incPaths
            ${${PROJECT_NAME}_incPaths}
            ${Qt5Gui_INCLUDE_DIRS}
            ${Qt5Svg_INCLUDE_DIRS}
            ${Qt5Widgets_INCLUDE_DIRS}
            ${Qt5Xml_INCLUDE_DIRS}
            "/usr/local/include"
        )
    endif()

    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "includes/editor"
        "../thirdparty/pugixml/src"
        "../thirdparty/assimp/include"
    )

    file(GLOB MOC_HEADERS
        "includes/viewport/*.h"
    )

    file(GLOB EDITOR_SOURCES
        "src/editor/*.cpp"
    )

    QT_WRAP_CPP(MOC_SRCS
        "includes/editor/asseteditor.h"
        "includes/editor/assetmanager.h"
        "includes/editor/baseassetprovider.h"
        "includes/editor/editorgadget.h"
        "includes/editor/editorplatform.h"
        "includes/editor/pluginmanager.h"
        "includes/editor/propertyedit.h"
        ${MOC_HEADERS}
    )

    # Dynamic Library
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles} ${MOC_SRCS} ${EDITOR_SOURCES})

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        assimp
        engine-editor
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Svg
    )

    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
        EDITOR_LIBRARY
    )

    if(UNIX AND NOT APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
        )

        # Solve build error using Clang on BSDs
        if(NOT LINUX)
            target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
        endif()
    endif()

    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    file(GLOB ${PROJECT_NAME}_editorFiles
        "src/editor/*.cpp"
        "src/converters/*.cpp"
        "src/viewport/*.cpp"
    )

    target_sources(${PROJECT_NAME}-editor PRIVATE
        "src/adapters/platformadaptor.cpp"
        "src/adapters/desktopadaptor.cpp"
        ${${PROJECT_NAME}_editorFiles}
    )

    if(WIN32)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE Shell32)
    elseif(APPLE)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE "-framework IOKit")
    endif()

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../${LIB_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}-editor
        DESTINATION "${LIB_PATH}"
    )

    install(DIRECTORY includes/editor/
        DESTINATION "${INC_PATH}/editor"
        FILES_MATCHING PATTERN "*.h"
        PATTERN "converters" EXCLUDE
    )

endif()

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../${STATIC_PATH}"
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION "${STATIC_PATH}"
)
