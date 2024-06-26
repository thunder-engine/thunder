cmake_minimum_required(VERSION 3.10)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)

project(uikit)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/components/*.cpp"
    "src/editor/*.cpp"
    "src/pipelinetasks/*.cpp"
    "src/resources/*.cpp"
    "src/utils/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "includes"
    "includes/resources"
    "../../common"
    "../../engine/includes"
    "../../engine/includes/resources"
    "../../engine/includes/components"
    "../../engine/includes/editor"
    "../../thirdparty/next/inc"
    "../../thirdparty/next/inc/math"
    "../../thirdparty/next/inc/core"
    "../../thirdparty/pugixml/src"
    "../../thirdparty/cssparser/include"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        include_directories(${Qt5Gui_INCLUDE_DIRS})
        include_directories(${Qt5Widgets_INCLUDE_DIRS})
        "/usr/local/include"
    )
endif()

file(GLOB ${PROJECT_NAME}_ui
    "src/editor/*.ui"
)

file(GLOB MOC_HEADERS
    "includes/converters/*.h"
    "includes/editor/*.h"
    "includes/tools/*.h"
)

set( RESOURCES
    src/editor/uieditor.qrc
)

QT5_ADD_RESOURCES(RES_SOURCES ${RESOURCES})
QT5_WRAP_UI(UI_HEADERS ${${PROJECT_NAME}_ui})
QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

add_definitions (
    -DCOMPANY_NAME="${COMPANY_NAME}"
    -DEDITOR_NAME="${EDITOR_NAME}"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

# Dynamic Library
if (desktop)
    add_library(${PROJECT_NAME}-editor SHARED
        ${${PROJECT_NAME}_srcFiles}
        ${MOC_SRCS}
        ${UI_HEADERS}
        ${RES_SOURCES}
        "src/converters/stylesheetconverter.cpp"
        "src/converters/uiconverter.cpp"
        "src/editor/uiedit.cpp"
        "src/editor/widgetcontroller.cpp"
        "src/editor/tools/widgettool.cpp"
    )

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        next-editor
        engine-editor
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
    )
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
        UIKIT_LIBRARY
    )

    # Solve build error using Clang on BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
    endif()

    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${PLUGINS_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${PLUGINS_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}-editor
            DESTINATION "${PLUGINS_PATH}"
    )

    install(FILES "includes/${PROJECT_NAME}.h"
            DESTINATION "${INC_PATH}/modules"
    )

    install(FILES
        "includes/components/abstractbutton.h"
        "includes/components/button.h"
        "includes/components/floatinput.h"
        "includes/components/frame.h"
        "includes/components/image.h"
        "includes/components/label.h"
        "includes/components/layout.h"
        "includes/components/menu.h"
        "includes/components/progressbar.h"
        "includes/components/recttransform.h"
        "includes/components/switch.h"
        "includes/components/textinput.h"
        "includes/components/toolbutton.h"
        "includes/components/uiloader.h"
        "includes/components/widget.h"
        DESTINATION "${INC_PATH}/engine"
    )
    
    install(FILES
        "includes/resources/stylesheet.h"
        "includes/resources/uidocument.h"
        DESTINATION "${INC_PATH}/engine"
    )

endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_link_libraries(${PROJECT_NAME} PRIVATE
    next
    engine
)
if (NOT desktop)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        THUNDER_MOBILE
    )
endif()

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
)

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${STATIC_PATH}"
)
