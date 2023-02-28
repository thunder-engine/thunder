# import qbs
# 
# Project {
#     id: physfs
#     property stringList srcFiles: {
#         var result = [
#             "src/*.c",
#             "src/archivers/*.c"
#         ];
#         if(qbs.targetOS.contains("windows")) {
#             result.push("src/platform/win32.c")
#         } else {
#             result.push("src/platform/unix.c"),
#             result.push("src/platform/posix.c")
#         }
#         return result;
#     }
# 
#     property stringList incPaths: [
#         "src",
#         "../zlib/src"
#     ]
# 
#     DynamicLibrary {
#         name: "physfs-editor"
#         condition: physfs.desktop
#         files: physfs.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         Depends { name: "zlib-editor" }
#         bundle.isBundle: false
# 
#         cpp.defines: ["PHYSFS_SUPPORTS_ZIP", "PHYSFS_NO_CDROM_SUPPORT"]
#         cpp.includePaths: physfs.incPaths
#         cpp.libraryPaths: [ ]
# 
#         Properties {
#             condition: qbs.targetOS.contains("windows")
#             cpp.dynamicLibraries: [ "Advapi32" ]
#         }
# 
#         Properties {
#             condition: qbs.targetOS.contains("darwin")
#             cpp.defines: outer.concat(["PHYSFS_DARWIN"])
#             cpp.sonamePrefix: "@executable_path"
#         }
# 
#         Group {
#             name: "Install Dynamic physfs"
#             fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
#             qbs.install: true
#             qbs.installDir: physfs.LIB_PATH + "/" + physfs.bundle
#             qbs.installPrefix: physfs.PREFIX
#         }
#     }
# 
#     StaticLibrary {
#         name: "physfs"
#         files: physfs.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.defines: ["PHYSFS_SUPPORTS_ZIP", "PHYSFS_NO_CDROM_SUPPORT"]
#         cpp.includePaths: physfs.incPaths
# 
#         Properties {
#             condition: qbs.targetOS.contains("darwin")
#             cpp.defines: outer.concat(["PHYSFS_DARWIN"])
#         }
# 
#         Group {
#             name: "Install Static physfs"
#             fileTagsFilter: product.type
#             qbs.install: true
#             qbs.installDir: physfs.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
#             qbs.installPrefix: physfs.PREFIX
#         }
#     }
# }

project(physfs)

file(GLOB ${PROJECT_NAME}_srcList
    src/*.c
    src/archivers/*.c
)

set(${PROJECT_NAME}_incPaths
    src
)

set(${PROJECT_NAME}_defines
    PHYSFS_SUPPORTS_ZIP
    PHYSFS_NO_CDROM_SUPPORT
)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    list(APPEND ${PROJECT_NAME}_defines
        PHYSFS_DARWIN
    )
endif()


add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME}-editor PUBLIC ${${PROJECT_NAME}_incPaths})
target_link_libraries(${PROJECT_NAME}-editor PUBLIC
    zlib-editor
)
target_compile_definitions(${PROJECT_NAME}-editor PUBLIC ${${PROJECT_NAME}_defines})


add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_incPaths})
target_link_libraries(${PROJECT_NAME} PUBLIC
    zlib
)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_defines})

