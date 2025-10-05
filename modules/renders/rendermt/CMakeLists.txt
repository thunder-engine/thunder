cmake_minimum_required(VERSION 3.10)

project(rendermt)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/components/*.cpp"
    "src/resources/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "includes"
    "../../../common"
    "../../../engine/includes"
    "../../../engine/includes/resources"
    "../../../engine/includes/components"
    "../../../thirdparty/next/inc"
    "../../../thirdparty/next/inc/math"
    "../../../thirdparty/next/inc/core"
    "../../../thirdparty/glfw/include"
    "../../../thirdparty/glfm/include"
    "../../../thirdparty/metal/metal-cpp",
    "../../../thirdparty/metal/metal-cpp-extensions"
)

if(APPLE)

# Dynamic Library
if(desktop)
    file(GLOB MOC_HEADERS
        "*.h"
        "includes/editor/*.h"
    )

    QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

    add_library(${PROJECT_NAME}-editor SHARED
        ${${PROJECT_NAME}_srcFiles}
        ${MOC_SRCS}
    )

    set(${PROJECT_NAME}_libs
        next-editor
        engine-editor
        glfw-editor
        Qt5::Core
        Qt5::Widgets
    )

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_libs})

    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
    )

    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../../${PLUGINS_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../../${PLUGINS_PATH}"
        FOLDER "modules"
    )

    install(TARGETS ${PROJECT_NAME}-editor
        DESTINATION "${PLUGINS_PATH}"
    )

    install(FILES "includes/${PROJECT_NAME}.h"
        DESTINATION "${INC_PATH}/modules"
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})

target_link_libraries(${PROJECT_NAME} PRIVATE
    next
    engine
    glfw
)

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

if(NOT desktop)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        THUNDER_MOBILE
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../../${STATIC_PATH}"
    FOLDER "modules"
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION "${STATIC_PATH}"
)

endif() # APPLE