cmake_minimum_required(VERSION 3.10)

project(tests)

file(GLOB ${PROJECT_NAME}_srcFiles
    "tests.cpp"
    "../thirdparty/next/tests/tst_*.h"
    "../engine/tests/tst_*.h"
    "../modules/network/tests/tst_*.h"
)

set(${PROJECT_NAME}_incPaths
    "../thirdparty/next/tests"
    "../thirdparty/next/inc"
    "../thirdparty/next/inc/core"
    "../thirdparty/next/inc/math"
    "../thirdparty/next/inc/anim"
    "../engine/tests"
    "../engine/includes"
    "../engine/includes/resources"
    "../engine/includes/components"
    "../modules/network/tests"
    "../modules/network/includes"
    "../modules/network/includes/objects"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
	set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
		"/usr/local/include"
	)
endif()

if (desktop)
    add_executable(${PROJECT_NAME}
        ${${PROJECT_NAME}_srcFiles}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        next-editor
        engine-editor
        network-editor
        GTest
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        SHARED_DEFINE
    )

    if(UNIX AND NOT APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
        )

        # Solve build error using Clang on BSDs
        if(NOT LINUX)
            target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
        endif()
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../${BIN_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../${BIN_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}
            DESTINATION "${BIN_PATH}"
    )
endif()
