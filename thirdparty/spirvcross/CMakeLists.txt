# import qbs
# import qbs.FileInfo
# 
# Project {
#     id: spirvcross
#     property stringList srcFiles: [
#         "src/*.cpp",
#         "src/*.hpp"
#     ]
# 
#     property stringList incPaths: [
#         "src"
#     ]
# 
#     StaticLibrary {
#         name: "spirvcross"
#         condition: spirvcross.desktop
#         files: spirvcross.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.defines: [ ]
#         cpp.includePaths: spirvcross.incPaths
#         cpp.cxxLanguageVersion: spirvcross.languageVersion
#         cpp.cxxStandardLibrary: spirvcross.standardLibrary
#         cpp.minimumMacosVersion: spirvcross.osxVersion
#     }
# }

project(spirvcross)

file(GLOB ${PROJECT_NAME}_srcList
    src/*.cpp
    src/*.hpp
)

set(${PROJECT_NAME}_incPaths
    src
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_incPaths})
