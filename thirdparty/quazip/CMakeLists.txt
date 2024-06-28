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

set(${PROJECT_NAME}_incPaths
	"src"
	"../zlib/src"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
	set(${PROJECT_NAME}_incPaths
		${${PROJECT_NAME}_incPaths}
		"/usr/local/include"
	)
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles} ${MOC_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE QUAZIP_BUILD NOMINMAX)

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

# Dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
