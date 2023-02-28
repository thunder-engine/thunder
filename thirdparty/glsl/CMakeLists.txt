# import qbs
# import qbs.FileInfo
# 
# Project {
#     id: glsl
#     property stringList srcFiles: [
#         "glslang/GenericCodeGen/**/*.cpp",
#         "glslang/MachineIndependent/**/*.cpp",
#         "glslang/Public/**/*.cpp",
#         "SPIRV/**/*.cpp",
#         "glslang/GenericCodeGen/**/*.h",
#         "glslang/MachineIndependent/**/*.h",
#         "glslang/Public/**/*.h",
#         "glslang/Include/**/*.h",
#         "glslang/OSDependent/**/*.h",
#         "SPIRV/**/*.h",
#         "OGLCompilersDLL/*.cpp"
#     ]
# 
#     property stringList incPaths: [
#         "./",
#         "OGLCompilersDLL"
#     ]
# 
#     StaticLibrary {
#         name: "glsl"
#         condition: glsl.desktop
#         files: glsl.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.includePaths: glsl.incPaths
#         cpp.libraryPaths: [ ]
#         cpp.cxxLanguageVersion: glsl.languageVersion
#         cpp.cxxStandardLibrary: glsl.standardLibrary
#         cpp.minimumMacosVersion: glsl.osxVersion
# 
#         Properties {
#             condition: qbs.targetOS.contains("windows")
#             files: outer.concat(["glslang/OSDependent/Windows/ossource.cpp"])
#         }
#         
#         Properties {
#             condition: qbs.targetOS.contains("unix")
#             files: outer.concat(["glslang/OSDependent/Unix/ossource.cpp"])
#         }
#     }
# }

project(glsl)

if(NOT (UNIX OR CMAKE_SYSTEM_NAME MATCHES "Windows"))
    message(-- Skipp ${PROJECT_NAME} for ${CMAKE_SYSTEM_NAME})
    return()
endif()

file(GLOB_RECURSE ${PROJECT_NAME}_srcList
    glslang/GenericCodeGen/**/*.cpp
    glslang/MachineIndependent/**/*.cpp
    glslang/Public/**/*.cpp
    SPIRV/**/*.cpp
    glslang/GenericCodeGen/**/*.h
    glslang/MachineIndependent/**/*.h
    glslang/Public/**/*.h
    glslang/Include/**/*.h
    glslang/OSDependent/**/*.h
    SPIRV/**/*.h
    OGLCompile
    rsDLL/*.cpp
)

if(UNIX)
    list(APPEND ${PROJECT_NAME}_srcList
        glslang/OSDependent/Unix/ossource.cpp
    ) 
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    list(APPEND ${PROJECT_NAME}_srcList
        glslang/OSDependent/Windows/ossource.cpp
    ) 
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})

target_include_directories(${PROJECT_NAME} PUBLIC
    .
    OGLCompilersDLL
)