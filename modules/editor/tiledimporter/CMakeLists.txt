cmake_minimum_required(VERSION 3.10)

project(tiledimporter)

file(GLOB ${PROJECT_NAME}_srcFiles
    "*.cpp"
    "converter/*.cpp"
    "converter/*.h"
)

set(${PROJECT_NAME}_incPaths
    "editor"
    "../../../engine/includes"
    "../../../engine/includes/resources"
    "../../../engine/includes/components"
    "../../../engine/includes/editor"
    "../../../thirdparty/next/inc"
    "../../../thirdparty/next/inc/math"
    "../../../thirdparty/next/inc/core"
)

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

# Dynamic Library
if (desktop)
    add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_srcFiles})

    target_link_libraries(${PROJECT_NAME} PRIVATE
        next-editor
        engine-editor
        Qt5::Core
        Qt5::Gui
        Qt5::Xml
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        SHARED_DEFINE
    )

    target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

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
