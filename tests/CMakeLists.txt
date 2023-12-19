cmake_minimum_required(VERSION 3.10)

project(tests)

file(GLOB ${PROJECT_NAME}_srcFiles
    "tests.cpp"
    "../thirdparty/next/tests/tst_*.h"
    "../engine/tests/tst_*.h"
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
)
if (desktop)
    add_executable(${PROJECT_NAME}
        ${${PROJECT_NAME}_srcFiles}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        next-editor
        engine-editor
        GTest
        Qt5::Core
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        SHARED_DEFINE
    )

    target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../${BIN_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../${BIN_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}
            DESTINATION "${BIN_PATH}"
    )
endif()
