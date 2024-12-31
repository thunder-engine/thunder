cmake_minimum_required(VERSION 3.10)

project(next)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/core/*.cpp"
    "src/math/*.cpp"
    "src/anim/*.cpp"
    "src/analytics/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "inc"
    "inc/core"
    "inc/math"
    "inc/anim"
    "inc/analytics"
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
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles})
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE SHARED_DEFINE NEXT_LIBRARY)
    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths})

    # macOS-specific configurations
    if(APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            MACOSX_RPATH TRUE
            SUFFIX ".dylib"
        )
        target_compile_definitions(${PROJECT_NAME}-editor PRIVATE NEXT_LIBRARY)
    endif()

    # Solve build error using Clang on BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        set(THREADS_PREFER_PTHREAD_FLAG TRUE)
        find_package(Threads)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE ${CMAKE_THREAD_LIBS_INIT})
        target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
    endif()

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
    )

    # Install
    install(TARGETS ${PROJECT_NAME}-editor
        LIBRARY DESTINATION ${LIB_PATH}
        ARCHIVE DESTINATION ${LIB_PATH}
        RUNTIME DESTINATION ${LIB_PATH}
    )

    # Install includes
    install(DIRECTORY inc/
        DESTINATION ${INC_PATH}/next
        FILES_MATCHING PATTERN "*.h"
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
)

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

# Install
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${STATIC_PATH}
    ARCHIVE DESTINATION ${STATIC_PATH}
)
