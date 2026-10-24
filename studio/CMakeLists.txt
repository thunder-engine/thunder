cmake_minimum_required(VERSION 3.10)

project(studio)

file(GLOB_RECURSE ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/*.h"
)

set(${PROJECT_NAME}_incPaths
    "src"
    "../"
    "../common"
    "../engine/includes"
    "../engine/includes/resources"
    "../engine/includes/components"
    "../thirdparty/glfw/include"
    "../editor/includes"
    "../modules/uikit/includes"
    "../modules/uikit/includes/components"
    "../modules/uikit/includes/resources"
    "../thirdparty/next/inc"
    "../thirdparty/next/inc/math"
    "../thirdparty/next/inc/core"
    "../thirdparty/next/inc/anim"
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
    -DSTUDIO_NAME="${STUDIO_NAME}"
    -DPRODUCT_NAME="${PRODUCT_NAME}"
    -DSDK_VERSION="${SDK_VERSION}"
    -DREVISION="${REVISION}"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

if(desktop)
    file(GLOB MOC_HEADERS
        "*.h"
    )

    QT_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

    add_executable(${STUDIO_NAME} ${${PROJECT_NAME}_srcFiles} ${MOC_SRCS})

    target_link_libraries(${STUDIO_NAME} PRIVATE
        next
        engine
        editor
        uikit-editor
    )

    target_compile_definitions(${STUDIO_NAME} PRIVATE
        SHARED_DEFINE
    )

    target_include_directories(${STUDIO_NAME} PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    if(UNIX AND NOT APPLE)
        set_target_properties(${STUDIO_NAME} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
        )

        # Solve build error using Clang on BSDs
        if(NOT LINUX)
            target_compile_options(${STUDIO_NAME} PRIVATE -fPIC)
        endif()
    endif()

    set_target_properties(${STUDIO_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../${BIN_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../${BIN_PATH}"
    )

    install(TARGETS ${STUDIO_NAME}
        DESTINATION "${BIN_PATH}"
    )

endif()
