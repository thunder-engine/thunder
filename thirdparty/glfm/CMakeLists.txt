# import qbs
# import qbs.FileInfo
# 
# Project {
#     id: glfm
#     property stringList srcFiles: {
#         var sources = [
#         "src/glfm_platform.h",
#         "include/glfm.h"
#         ];
#         return sources;
#     }
# 
#     property stringList incPaths: [
#         "include"
#     ]
# 
#     StaticLibrary {
#         name: "glfm"
#         condition: !glfm.desktop
#         files: glfm.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.defines: [ ]
#         cpp.includePaths: glfm.incPaths
#         cpp.cLanguageVersion: "c11"
# 
#         Properties {
#             condition: qbs.targetOS.contains("android")
#             property string nativePath: FileInfo.joinPaths(Android.ndk.ndkDir, "sources", "android", "native_app_glue")
#             files: outer.concat([
#                 "src/glfm_platform_android.c",
#                 nativePath + "/android_native_app_glue.c",
#                 nativePath + "/android_native_app_glue.h"
#             ])
# 
#             cpp.includePaths: outer.concat([nativePath])
#             Android.ndk.appStl: glfm.ANDROID_STL
#             Android.ndk.platform: glfm.ANDROID
#         }
# 
#         Properties {
#             condition: qbs.targetOS.contains("ios") || qbs.targetOS.contains("tvos")
#             files: outer.concat(["src/glfm_platform_ios.m"])
#         }
# 
#         Group {
#             name: "Install Static glfm"
#             fileTagsFilter: product.type
#             qbs.install: true
#             qbs.installDir: glfm.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
#             qbs.installPrefix: glfm.PREFIX
#         }
#     }
# }

# TODO We have some version of glfm on github with cmake supprt. May we update version from github?

project(glfm)

if(NOT CMAKE_SYSTEM_NAME MATCHES "Android|Darwin")
    message("-- Skipp ${PROJECT_NAME} for ${CMAKE_SYSTEM_NAME}")
    return()
endif()

set(${PROJECT_NAME}_srcList 
    src/glfm_platform.h
    include/glfm.h
)

if (CMAKE_SYSTEM_NAME MATCHES "Android")
string(APPEND ${PROJECT_NAME}_srcList
    src/glfm_platform_android.c
    src/android_native_app_glue.c
    src/android_native_app_glue.h
)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
string(APPEND ${PROJECT_NAME}_srcList
    src/glfm_platform_ios.m
)    
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME} PUBLIC include)


