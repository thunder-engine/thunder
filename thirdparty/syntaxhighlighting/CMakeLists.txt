cmake_minimum_required(VERSION 3.10)

project(syntaxhighlighting)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
)

file(GLOB MOC_HEADERS
    "src/*.h"
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

set(RESOURCES
    syntax/syntax-data.qrc
    themes/theme-data.qrc
)

QT5_ADD_RESOURCES(RES_SOURCES ${RESOURCES})
QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles} ${MOC_SRCS} ${RES_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${${PROJECT_NAME}_incPaths}
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_SYNTAX_RESOURCE SyntaxHighlighting_VERSION_MAJOR=5 SyntaxHighlighting_VERSION_MINOR=57)

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

# Qt Dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Network)
