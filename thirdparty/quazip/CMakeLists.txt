# import qbs
# 
# Project {
#     id: quazip
#     property stringList srcFiles: [
#         "src/*.c",
#         "src/*.cpp",
#         "src/*.h"
#     ]
# 
#     property stringList incPaths: [
#         "src",
#         "../zlib/src"
#     ]
# 
#     StaticLibrary {
#         name: "quazip"
#         condition: quazip.desktop
#         files: quazip.srcFiles
# 
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         Depends { name: "zlib-editor" }
#         Depends { name: "Qt"; submodules: ["core"]; }
#         bundle.isBundle: false
# 
#         cpp.defines: ["QUAZIP_BUILD", "NOMINMAX"]
#         cpp.includePaths: quazip.incPaths
#         cpp.libraryPaths: [ ]
#         cpp.dynamicLibraries: [ ]
#     }
# }

project(quazip)

file(GLOB ${PROJECT_NAME}_srcList 
    src/*.c
    src/*.cpp
    src/*.h
)

find_package(Qt5 COMPONENTS Core REQUIRED)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(${PROJECT_NAME} PUBLIC src)

target_link_libraries(${PROJECT_NAME} PUBLIC
    zlib-editor
    Qt5::Core
)
