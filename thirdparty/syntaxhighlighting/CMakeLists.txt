# import qbs
# 
# Project {
#     id: syntaxhighlighting
#     property stringList srcFiles: [
#         "src/*.cpp",
#         "src/*.h",
#         "themes/*.qrc",
#         "syntax/*.qrc"
#     ]
# 
#     property stringList incPaths: [
#         "src"
#     ]
# 
#     StaticLibrary {
#         name: "syntaxhighlighting"
#         condition: syntaxhighlighting.desktop
#         files: syntaxhighlighting.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         Depends { name: "Qt"; submodules: ["core", "gui", "network"]; }
#         bundle.isBundle: false
# 
#         cpp.defines: [ "HAS_SYNTAX_RESOURCE", "SyntaxHighlighting_VERSION_MAJOR=5", "SyntaxHighlighting_VERSION_MINOR=57" ]
#         cpp.includePaths: syntaxhighlighting.incPaths
#     }
# }


project(syntaxhighlighting)

find_package(Qt5 COMPONENTS Core Gui Network REQUIRED)

file(GLOB ${PROJECT_NAME}_srcList
    src/*.cpp
    src/*.h
    themes/*.qrc
    syntax/*.qrc
)

set(${PROJECT_NAME}_incPaths
    src
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_incPaths})
target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Network
)
target_compile_definitions(${PROJECT_NAME} PUBLIC
    HAS_SYNTAX_RESOURCE
    SyntaxHighlighting_VERSION_MAJOR=5
    SyntaxHighlighting_VERSION_MINOR=57
)

