cmake_minimum_required(VERSION 3.10)

project(navigation)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/components/*.cpp"
    "src/resources/*.cpp"
    "src/utils/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "editor"
    "includes"
    "../physics/bullet/includes"
    "../physics/bullet/includes/components"
    "../physics/bullet/includes/resources"
    "../../engine/includes"
    "../../engine/includes/components"
    "../../engine/includes/resources"
    "../../editor/includes"
    "../../thirdparty/next/inc"
    "../../thirdparty/next/inc/math"
    "../../thirdparty/next/inc/core"
    "../../thirdparty/recast/Recast/Include"
    "../../thirdparty/recast/Detour/Include"
    "../../thirdparty/recast/DetourCrowd/Include"
    "../../thirdparty/recast/DetourTileCache/Include"
    "../../thirdparty/recast/DebugUtils/Include"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

# Dynamic Library
if(desktop)
    file(GLOB ${PROJECT_NAME}_editorFiles
        "editor/*.cpp"
        "editor/*.h"
        "editor/*.ui"
    )

    set(CMAKE_AUTOUIC ON)

    add_library(${PROJECT_NAME}-editor SHARED
        ${${PROJECT_NAME}_srcFiles}
        ${${PROJECT_NAME}_editorFiles}
    )

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        next-editor
        engine-editor
        bullet-editor
        editor
        recast
    )
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
        NAVIGATION_LIBRARY
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

    target_include_directories(${PROJECT_NAME}-editor PRIVATE
        ${${PROJECT_NAME}_incPaths}
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${PLUGINS_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${PLUGINS_PATH}"
        FOLDER "modules"
    )

    install(TARGETS ${PROJECT_NAME}-editor
        DESTINATION "${PLUGINS_PATH}"
    )

    install(FILES "includes/${PROJECT_NAME}.h"
        DESTINATION "${INC_PATH}/modules"
    )

    file(GLOB ${PROJECT_NAME}_headers
        "includes/components/*.h"
        "includes/resources/*.h"
    )

    install(FILES ${${PROJECT_NAME}_headers}
        DESTINATION "${INC_PATH}/engine"
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})

if(NOT desktop)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        THUNDER_MOBILE
    )
endif()

if(UNIX AND NOT APPLE)
    # Solve build error using Clang on BSDs
    if(NOT LINUX)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
    endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
    FOLDER "modules"
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION "${STATIC_PATH}"
)

install(FILES "includes/${PROJECT_NAME}.h"
    DESTINATION "${INC_PATH}/modules"
)

file(GLOB ${PROJECT_NAME}_headers
    "includes/components/*.h"
    "includes/resources/*.h"
)

install(FILES ${${PROJECT_NAME}_headers}
    DESTINATION "${INC_PATH}/engine"
)
