cmake_minimum_required(VERSION 3.10)

project(pugixml)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/*.h"
)

# Static Library
add_library(pugixml STATIC ${${PROJECT_NAME}_srcFiles})
target_include_directories(${PROJECT_NAME} PRIVATE "src")
