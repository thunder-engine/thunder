cmake_minimum_required(VERSION 3.10)

project(quazip)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.c"
    "src/*.cpp"
)

file(GLOB MOC_HEADERS
    "src/*.h"
)

QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles} ${MOC_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE "src" "../zlib/src" ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE QUAZIP_BUILD NOMINMAX)

# Dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
