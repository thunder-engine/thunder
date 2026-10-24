cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOUIC ON)

file(GLOB uikit_formeditor_srcFiles
    "../src/converters/*.cpp"
    "../src/editor/*.cpp"
    "../src/editor/tools/*.cpp"
    "../src/editor/actions/*.cpp"
    "../../../thirdparty/pugixml/src/*.cpp"
    "formeditor.cpp"
)

file(GLOB uikit_formeditor_ui
    "../src/editor/*.ui"
)

file(GLOB uikit_formeditor_mocHeaders
    "../includes/converters/*.h"
    "../includes/editor/*.h"
    "../includes/editor/tools/*.h"
)

QT_ADD_RESOURCES(uikit_formeditor_resources ../src/editor/uieditor.qrc)
QT_WRAP_UI(uikit_formeditor_uiHeaders ${uikit_formeditor_ui})
QT_WRAP_CPP(uikit_formeditor_mocSources ${uikit_formeditor_mocHeaders})

add_library(uikit-formeditor SHARED
    ${uikit_formeditor_srcFiles}
    ${uikit_formeditor_resources}
    ${uikit_formeditor_uiHeaders}
    ${uikit_formeditor_mocSources}
)

target_link_libraries(uikit-formeditor PRIVATE
    next-editor
    engine-editor
    uikit-editor
    editor
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

target_compile_definitions(uikit-formeditor PRIVATE
    SHARED_DEFINE
)

target_include_directories(uikit-formeditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../includes
    ${CMAKE_CURRENT_SOURCE_DIR}/../includes/components
    ${CMAKE_CURRENT_SOURCE_DIR}/../includes/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/../includes/editor
    ${CMAKE_CURRENT_SOURCE_DIR}/../includes/editor/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../engine/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../engine/includes/components
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../engine/includes/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../editor/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../thirdparty/next/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../thirdparty/next/inc/math
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../thirdparty/next/inc/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../thirdparty/pugixml/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../thirdparty/cssparser/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(uikit-formeditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../../${LIB_PATH}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../../${LIB_PATH}"
    FOLDER "modules"
)

install(TARGETS uikit-formeditor DESTINATION "${LIB_PATH}")

if(UNIX AND NOT APPLE)
    set_target_properties(uikit-formeditor PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
    )

    if(NOT LINUX)
        target_compile_options(uikit-formeditor PRIVATE -fPIC)
    endif()
endif()

if(APPLE)
    set_target_properties(uikit-formeditor PROPERTIES
        INSTALL_NAME_DIR "@executable_path"
    )
endif()
