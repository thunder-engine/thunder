cmake_minimum_required(VERSION 3.10)

project(poly2tri)

file(GLOB ${PROJECT_NAME}_srcFiles
    "poly2tri/**/*.cpp"
    "poly2tri/**/*.h"
)

set(${PROJECT_NAME}_incPaths
    ""
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
