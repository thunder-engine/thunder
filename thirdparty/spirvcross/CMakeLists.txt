cmake_minimum_required(VERSION 3.10)

project(spirvcross)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/*.hpp"
)

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_include_directories(${PROJECT_NAME} PRIVATE "src")
target_compile_definitions(${PROJECT_NAME} PRIVATE)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /wd4267 /wd4996 $<$<CONFIG:DEBUG>:/bigobj>)
endif()