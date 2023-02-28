# import qbs
# 
# Project {
#     id: poly2tri
#     property stringList srcFiles: [
#         "poly2tri/**/*.cpp",
#         "poly2tri/**/*.h"
#     ]
# 
#     property stringList incPaths: [
#     ]
# 
#     StaticLibrary {
#         name: "poly2tri"
#         files: poly2tri.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.includePaths: poly2tri.incPaths
#         cpp.cxxLanguageVersion: poly2tri.languageVersion
#         cpp.cxxStandardLibrary: poly2tri.standardLibrary
#         cpp.minimumMacosVersion: poly2tri.osxVersion
#     }
# }
project(poly2tri)

file(GLOB ${PROJECT_NAME}_srcList
    poly2tri/**/*.cc
    poly2tri/**/*.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME} PUBLIC .)
