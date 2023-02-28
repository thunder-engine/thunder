# import qbs
# 
# Project {
#     id: irrxml
#     property stringList srcFiles: [
#         "src/**/*.cpp",
#         "src/**/*.h"
#     ]
# 
#     property stringList incPaths: [
#         "src"
#     ]
# 
#     StaticLibrary {
#         name: "irrxml"
#         files: irrxml.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.includePaths: irrxml.incPaths
#         cpp.cxxLanguageVersion: irrxml.languageVersion
#         cpp.cxxStandardLibrary: irrxml.standardLibrary
#         cpp.minimumMacosVersion: irrxml.osxVersion
#     }
# }

project(irrxml)

file(GLOB_RECURSE ${PROJECT_NAME}_srcList
    src/*.cpp
    src/*.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})

target_include_directories(${PROJECT_NAME} PUBLIC src)
