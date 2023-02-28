# import qbs
# 
# Project {
#     id: assimp
#     property stringList srcFiles: [
#         "code/**/*.cpp",
#         "code/**/*.h"
#     ]
# 
#     property stringList incPaths: [
#         "./",
#         "include",
#         "code",
#         "../irrXML/src",
#         "../poly2tri",
#         "../zlib/src",
#         "../utf8cpp",
#         "../quazip/src"
#     ]
# 
#     StaticLibrary {
#         name: "assimp"
#         condition: assimp.desktop
# 
#         cpp.defines: [
#             "ASSIMP_BUILD_NO_OWN_ZLIB",
#             "ASSIMP_USE_HUNTER",
#             "ASSIMP_BUILD_NO_GLTF_IMPORTER",
#             "ASSIMP_BUILD_NO_IFC_IMPORTER",
#             "ASSIMP_BUILD_NO_OPENGEX_IMPORTER",
#             "ASSIMP_BUILD_NO_C4D_IMPORTER",
#             "ASSIMP_BUILD_NO_3MF_EXPORTER",
#             "ASSIMP_BUILD_NO_STEP_IMPORTER"]
# 
#         files: assimp.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "irrxml" }
#         Depends { name: "quazip" }
# 
#         cpp.includePaths: assimp.incPaths
#         cpp.cxxLanguageVersion: assimp.languageVersion
#         cpp.cxxStandardLibrary: assimp.standardLibrary
#         cpp.minimumMacosVersion: assimp.osxVersion
#     }
# }

project(assimp)

file(GLOB ${PROJECT_NAME}_srcList
    code/**/*.cpp
    code/**/*.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE code)
target_compile_definitions(${PROJECT_NAME} PUBLIC # TODO Set all defines publick by defualt, but need check
    ASSIMP_BUILD_NO_OWN_ZLIB
    ASSIMP_USE_HUNTER
    ASSIMP_BUILD_NO_GLTF_IMPORTER
    ASSIMP_BUILD_NO_IFC_IMPORTER
    ASSIMP_BUILD_NO_OPENGEX_IMPORTER
    ASSIMP_BUILD_NO_C4D_IMPORTER
    ASSIMP_BUILD_NO_3MF_EXPORTER
    ASSIMP_BUILD_NO_STEP_IMPORTER
)
target_link_libraries(${PROJECT_NAME}
PUBLIC
    irrxml
    quazip
PRIVATE
    poly2tri
    utf8cpp
)
