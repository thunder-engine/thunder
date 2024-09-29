cmake_minimum_required(VERSION 3.10)

project(angel)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/bindings/*.cpp"
    "src/components/*.cpp"
    "src/resources/*.cpp"
)

file(GLOB ${PROJECT_NAME}_modules
    "../../../thirdparty/angelscript/modules/debugger/*.cpp"
    "../../../thirdparty/angelscript/modules/scriptarray/*.cpp"
    "../../../thirdparty/angelscript/modules/scriptdictionary/*.cpp"
    "../../../thirdparty/angelscript/modules/scriptgrid/*.cpp"
    "../../../thirdparty/angelscript/modules/scriptmath/*.cpp"
    "../../../thirdparty/angelscript/modules/scriptstdstring/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "includes"
    "../../../common"
    "../../../thirdparty/next/inc"
    "../../../thirdparty/next/inc/math"
    "../../../thirdparty/next/inc/core"
    "../../../engine/includes"
    "../../../engine/includes/resources"
    "../../../engine/includes/components"
    "../../../engine/includes/editor"
    "../../../thirdparty/angelscript/include"
    "../../../thirdparty/angelscript/modules"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

# Dynamic Library
if(desktop)
    file(GLOB MOC_HEADERS
        "includes/converters/*.h"
    )

    set(RESOURCES
        src/converters/templates.qrc
    )

    QT5_ADD_RESOURCES(RES_SOURCES ${RESOURCES})
    QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

    add_library(${PROJECT_NAME}-editor SHARED
        ${${PROJECT_NAME}_srcFiles}
        ${${PROJECT_NAME}_modules}
        ${RES_SOURCES}
        ${MOC_SRCS}
        "src/converters/angelbuilder.cpp"
    )

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        next-editor
        engine-editor
        angelscript-editor
        Qt5::Core
        Qt5::Gui
    )

    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths})

    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
    )

    if(UNIX AND NOT APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            INSTALL_RPATH "$ORIGIN/../../lib"
        )

        # Solve build error using Clang on BSDs
        if(NOT LINUX)
            target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
        endif()
    endif()

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../../${PLUGINS_PATH}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../../${PLUGINS_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}-editor
            DESTINATION "${PLUGINS_PATH}"
    )

    install(FILES "includes/${PROJECT_NAME}.h"
            DESTINATION "${INC_PATH}/modules"
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_srcFiles}
    ${${PROJECT_NAME}_modules}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC -pthread)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    next
    engine
    angelscript
)

if(NOT desktop)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        THUNDER_MOBILE
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../../${STATIC_PATH}"
)

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${STATIC_PATH}"
)
