cmake_minimum_required(VERSION 3.10)

project(poly2tri)

file(GLOB ${PROJECT_NAME}_srcFiles
    "poly2tri/**/*.cpp"
    "poly2tri/**/*.h"
)

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_include_directories(${PROJECT_NAME} PRIVATE "")

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
