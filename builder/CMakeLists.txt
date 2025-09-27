cmake_minimum_required(VERSION 3.10)

project(builder)

file(GLOB ${PROJECT_NAME}_srcFiles
    "*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "src"
    "../"
    "../common"
    "../engine/includes"
    "../engine/includes/resources"
    "../engine/includes/components"
    "../thirdparty/next/inc"
    "../thirdparty/next/inc/math"
    "../thirdparty/next/inc/core"
    "../thirdparty/next/inc/anim"
    "../thirdparty/zlib/src"
    "../thirdparty/minizip"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

add_definitions (
    -DCOMPANY_NAME="${COMPANY_NAME}"
    -DBUILDER_NAME="${BUILDER_NAME}"
    -DSDK_VERSION="${SDK_VERSION}"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

if(desktop)
    file(GLOB MOC_HEADERS
        "*.h"
    )

    QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

    add_executable(${BUILDER_NAME} ${${PROJECT_NAME}_srcFiles} ${MOC_SRCS})

    target_link_libraries(${BUILDER_NAME} PRIVATE
        zlib-editor
        next-editor
        engine-editor
        minizip
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
    )

    target_compile_definitions(${BUILDER_NAME} PRIVATE
        SHARED_DEFINE
        BUILDER_NAME
    )

    target_include_directories(${BUILDER_NAME} PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    if(UNIX AND NOT APPLE)
        set_target_properties(${BUILDER_NAME} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
        )

        # Solve build error using Clang on BSDs
        if(NOT LINUX)
            target_compile_options(${BUILDER_NAME} PRIVATE -fPIC)
        endif()
    endif()

    set_target_properties(${BUILDER_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../${BIN_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../${BIN_PATH}"
    )

    install(TARGETS ${BUILDER_NAME}
        DESTINATION "${BIN_PATH}"
    )

endif()
