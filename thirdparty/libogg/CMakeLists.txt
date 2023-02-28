# import qbs
# 
# Project {
#     id: ogg
#     property stringList srcFiles: [
#         "src/*.c"
#     ]
# 
#     property stringList incPaths: [
#         "src",
#         "src/ogg"
#     ]
# 
#     DynamicLibrary {
#         name: "ogg-editor"
#         condition: ogg.desktop
#         files: ogg.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.defines: ["LIBOGG_LIBRARY"]
#         cpp.includePaths: ogg.incPaths
#         cpp.libraryPaths: [ ]
#         cpp.dynamicLibraries: [ ]
# 
#         Properties {
#             condition: qbs.targetOS.contains("windows") && !qbs.toolchain.contains("gcc")
#             cpp.linkerFlags: ["/DEF:" + path + "/src/ogg.def"]
#         }
# 
#         Properties {
#             condition: qbs.targetOS.contains("darwin")
#             cpp.sonamePrefix: "@executable_path"
#         }
# 
#         Group {
#             name: "Install Dynamic Ogg"
#             fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
#             qbs.install: true
#             qbs.installDir: ogg.LIB_PATH + "/" + ogg.bundle
#             qbs.installPrefix: ogg.PREFIX
#         }
#     }
# 
#     StaticLibrary {
#         name: "ogg"
#         files: ogg.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.defines: [  ]
#         cpp.includePaths: ogg.incPaths
# 
#         Group {
#             name: "Install Static Ogg"
#             condition: ogg.desktop
#             fileTagsFilter: product.type
#             qbs.install: true
#             qbs.installDir: ogg.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
#             qbs.installPrefix: ogg.PREFIX
#         }
#     }
# }

project(ogg)

file(GLOB ${PROJECT_NAME}_srcList
    src/*.c
)

set(${PROJECT_NAME}_incPaths
    src
    src/ogg
)


add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME}-editor PUBLIC ${${PROJECT_NAME}_incPaths})
target_compile_definitions(${PROJECT_NAME}-editor PUBLIC LIBOGG_LIBRARY)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_options(${PROJECT_NAME} PRIVATE /DEF${CMAKE_CURRENT_SOURCE_DIR}/src/ogg.def)
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_incPaths})

