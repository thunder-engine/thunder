cmake_minimum_required(VERSION 3.10)

project(bullet)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/components/*.cpp"
    "src/resources/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "includes"
    "includes/resources"
    "includes/components"
    "../../../common"
    "../../../thirdparty/next/inc"
    "../../../thirdparty/next/inc/math"
    "../../../thirdparty/next/inc/core"
    "../../../engine/includes"
    "../../../engine/includes/components"
    "../../../engine/includes/resources"
    "../../../engine/includes/editor"
    "../../../thirdparty/bullet/src"
)

file(GLOB MOC_HEADERS
    "includes/converters/*.h"
)

set(RESOURCES
    src/converters/templates.qrc
)

QT5_ADD_RESOURCES(RES_SOURCES ${RESOURCES})
QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

# Dynamic Library
if (desktop)
    add_library(${PROJECT_NAME}-editor SHARED
        ${${PROJECT_NAME}_srcFiles}
        ${RES_SOURCES}
        ${MOC_SRCS}
        "src/converters/physicmaterialconverter.cpp"
    )

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        next-editor
        engine-editor
        bullet3
        Qt5::Core
        Qt5::Gui
    )

    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
        BULLET_LIBRARY
    )

    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths})

    file(GLOB ${PROJECT_NAME}_headers
        "includes/components/*.h"
        "includes/resources/*.h"
    )

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

    install(FILES ${${PROJECT_NAME}_headers}
            DESTINATION "${INC_PATH}/engine"
    )
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

target_link_libraries(${PROJECT_NAME} PRIVATE
    bullet3
)

if (NOT desktop)
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
