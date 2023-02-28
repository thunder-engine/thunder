# import qbs
# 
# Project {
#     id: freetype
#     property stringList srcFiles: [
#         "src/base/ftbase.c",
#         "src/base/ftinit.c",
#         "src/base/ftsystem.c",
#         "src/base/ftbbox.c",
#         "src/base/ftbdf.c",
#         "src/base/ftbitmap.c",
#         "src/base/ftcid.c",
#         "src/base/ftdebug.c",
#         "src/base/ftfstype.c",
#         "src/base/ftgasp.c",
#         "src/base/ftglyph.c",
#         "src/base/ftgxval.c",
#         "src/base/ftmm.c",
#         "src/base/ftotval.c",
#         "src/base/ftpatent.c",
#         "src/base/ftpfr.c",
#         "src/base/ftstroke.c",
#         "src/base/ftsynth.c",
#         "src/base/fttype1.c",
#         "src/base/ftwinfnt.c",
#         "src/autofit/autofit.c",
#         "src/bdf/bdf.c",
#         "src/cff/cff.c",
#         "src/cache/ftcache.c",
#         "src/gzip/ftgzip.c",
#         "src/lzw/ftlzw.c",
#         "src/pcf/pcf.c",
#         "src/pfr/pfr.c",
#         "src/psaux/psaux.c",
#         "src/pshinter/pshinter.c",
#         "src/psnames/psmodule.c",
#         "src/raster/raster.c",
#         "src/sdf/sdf.c",
#         "src/sfnt/sfnt.c",
#         "src/smooth/smooth.c",
#         "src/svg/svg.c",
#         "src/truetype/truetype.c",
#         "src/type1/type1.c",
#         "src/cid/type1cid.c",
#         "src/type42/type42.c",
#         "src/winfonts/winfnt.c"
#     ]
# 
#     property stringList incPaths: [
#         "include",
#         "include/freetype"
#     ]
# 
#     DynamicLibrary {
#         name: "freetype-editor"
#         condition: freetype.desktop
#         files: freetype.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.defines: ["FT2_BUILD_LIBRARY", "DLL_EXPORT"]
#         cpp.includePaths: freetype.incPaths
#         cpp.libraryPaths: [ ]
#         cpp.dynamicLibraries: [ ]
# 
#         Properties {
#             condition: qbs.targetOS.contains("darwin")
#             cpp.sonamePrefix: "@executable_path"
#         }
# 
#         Group {
#             name: "Install Dynamic Platform"
#             fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
#             qbs.install: true
#             qbs.installDir: freetype.LIB_PATH + "/" + freetype.bundle
#             qbs.installPrefix: freetype.PREFIX
#         }
# 
#     }
# 
#     StaticLibrary {
#         name: "freetype"
#         files: freetype.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.defines: ["FT2_BUILD_LIBRARY"]
#         cpp.includePaths: freetype.incPaths
# 
#         Properties {
#             condition: qbs.targetOS.contains("android")
#             Android.ndk.appStl: freetype.ANDROID_STL
#             Android.ndk.platform: freetype.ANDROID
#         }
# 
#         Group {
#             name: "Install Static Freetype"
#             fileTagsFilter: product.type
#             qbs.install: true
#             qbs.installDir: freetype.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
#             qbs.installPrefix: freetype.PREFIX
#         }
#     }
# }
project(freetype)


file(GLOB_RECURSE ${PROJECT_NAME}_srcList
    src/base/ftbase.c
    src/base/ftinit.c
    src/base/ftsystem.c
    src/base/ftbbox.c
    src/base/ftbdf.c
    src/base/ftbitmap.c
    src/base/ftcid.c
    src/base/ftdebug.c
    src/base/ftfstype.c
    src/base/ftgasp.c
    src/base/ftglyph.c
    src/base/ftgxval.c
    src/base/ftmm.c
    src/base/ftotval.c
    src/base/ftpatent.c
    src/base/ftpfr.c
    src/base/ftstroke.c
    src/base/ftsynth.c
    src/base/fttype1.c
    src/base/ftwinfnt.c
    src/autofit/autofit.c
    src/bdf/bdf.c
    src/cff/cff.c
    src/cache/ftcache.c
    src/gzip/ftgzip.c
    src/lzw/ftlzw.c
    src/pcf/pcf.c
    src/pfr/pfr.c
    src/psaux/psaux.c
    src/pshinter/pshinter.c
    src/psnames/psmodule.c
    src/raster/raster.c
    src/sdf/sdf.c
    src/sfnt/sfnt.c
    src/smooth/smooth.c
    src/svg/svg.c
    src/truetype/truetype.c
    src/type1/type1.c
    src/cid/type1cid.c
    src/type42/type42.c
    src/winfonts/winfnt.c
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME} PUBLIC
    include
    include/freetype
)
target_compile_definitions(${PROJECT_NAME} PUBLIC # TODO Set all defines publick by defualt, but need check
    FT2_BUILD_LIBRARY
)

add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME}-editor PUBLIC
    include
    include/freetype
)
target_compile_definitions(${PROJECT_NAME}-editor PUBLIC # TODO Set all defines publick by defualt, but need check
    FT2_BUILD_LIBRARY
    DLL_EXPORT
)
