cmake_minimum_required(VERSION 3.10)

project(spirvcross)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/*.hpp"
)

set(${PROJECT_NAME}_incPaths
    "src"
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
target_compile_definitions(${PROJECT_NAME} PRIVATE)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4267 /wd4996 $<$<CONFIG:DEBUG>:/bigobj>)
endif()

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()
