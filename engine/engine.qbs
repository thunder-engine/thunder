import qbs

Project {
    id: engine
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/**/*.cpp",
        "src/resources/*.cpp",
        "src/systems/*.cpp",
        "src/filters/*.cpp",
        "src/pipelinetasks/*.cpp",
        "src/utils/*.cpp",
        "src/adapters/platform*.cpp",
        "../thirdparty/freetype/src/base/*tbase.c",
        "../thirdparty/freetype/src/base/*tinit.c",
        "../thirdparty/freetype/src/base/*tsystem.c",
        "../thirdparty/freetype/src/base/*tbbox.c",
        "../thirdparty/freetype/src/base/*tbdf.c",
        "../thirdparty/freetype/src/base/*tbitmap.c",
        "../thirdparty/freetype/src/base/*tcid.c",
        "../thirdparty/freetype/src/base/*tdebug.c",
        "../thirdparty/freetype/src/base/*tfstype.c",
        "../thirdparty/freetype/src/base/*tgasp.c",
        "../thirdparty/freetype/src/base/*tglyph.c",
        "../thirdparty/freetype/src/base/*tgxval.c",
        "../thirdparty/freetype/src/base/*tmm.c",
        "../thirdparty/freetype/src/base/*totval.c",
        "../thirdparty/freetype/src/base/*tpatent.c",
        "../thirdparty/freetype/src/base/*tpfr.c",
        "../thirdparty/freetype/src/base/*tstroke.c",
        "../thirdparty/freetype/src/base/*tsynth.c",
        "../thirdparty/freetype/src/base/*ttype1.c",
        "../thirdparty/freetype/src/base/*twinfnt.c",
        "../thirdparty/freetype/src/autofit/*utofit.c",
        "../thirdparty/freetype/src/bdf/*df.c",
        "../thirdparty/freetype/src/cff/*ff.c",
        "../thirdparty/freetype/src/cache/*tcache.c",
        "../thirdparty/freetype/src/lzw/*tlzw.c",
        "../thirdparty/freetype/src/pcf/*cf.c",
        "../thirdparty/freetype/src/pfr/*fr.c",
        "../thirdparty/freetype/src/psaux/*saux.c",
        "../thirdparty/freetype/src/pshinter/*shinter.c",
        "../thirdparty/freetype/src/psnames/*smodule.c",
        "../thirdparty/freetype/src/raster/raste*.c",
        "../thirdparty/freetype/src/sdf/sd*.c",
        "../thirdparty/freetype/src/sfnt/sfn*.c",
        "../thirdparty/freetype/src/smooth/smoot*.c",
        "../thirdparty/freetype/src/svg/sv*.c",
        "../thirdparty/freetype/src/truetype/*ruetype.c",
        "../thirdparty/freetype/src/type1/*ype1.c",
        "../thirdparty/freetype/src/cid/*ype1cid.c",
        "../thirdparty/freetype/src/type42/*ype42.c",
        "../thirdparty/freetype/src/winfonts/*infnt.c",
        "../thirdparty/physfs/src/*.c"
    ]

    property stringList incPaths: [
        "includes",
        "includes/components",
        "includes/resources",
        "includes/adapters",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/next/inc/anim",
        "../thirdparty/next/inc/os",
        "../thirdparty/physfs/src",
        "../thirdparty/glfw/include",
        "../thirdparty/glfm/include",
        "../thirdparty/freetype/include",
        "../thirdparty/pugixml/src",
        "../thirdparty/metal/metal-cpp",
        "../thirdparty/metal/metal-cpp-extensions"
    ]

    DynamicLibrary {
        name: "engine-editor"
        condition: engine.desktop
        files: {
            var result = engine.srcFiles
            if(qbs.targetOS.contains("darwin")) {
                result.push("../thirdparty/physfs/src/*.m")
            }
            return result
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "glfw-editor" }
        bundle.isBundle: false

        cpp.defines: {
            var result = engine.defines
            result.push("SHARED_DEFINE")
            result.push("ENGINE_LIBRARY")
            result.push("FT2_BUILD_LIBRARY")
            result.push("PHYSFS_SUPPORTS_ZIP")
            result.push("PHYSFS_SUPPORTS_DEFAULT=0")
            result.push("PHYSFS_NO_CDROM_SUPPORT")
            return result
        }
        cpp.includePaths: engine.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: engine.languageVersion
        cpp.cxxStandardLibrary: engine.standardLibrary
        cpp.minimumMacosVersion: engine.osxVersion

        Properties {
            condition: engine.desktop
            files: outer.concat(["src/adapters/desktopadaptor.cpp"])
        }

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: outer.concat([
                "Shell32", "Advapi32"
            ])
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["IOKit", "Foundation"]
            cpp.defines: outer.concat(["PHYSFS_DARWIN"])
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic Engine"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: engine.LIB_PATH
            qbs.installPrefix: engine.PREFIX
        }

        Group {
            name: "Engine includes"
            prefix: "includes/"
            files: [
                "*.h",
                "components/*.h",
                "components/gui/*.h",
                "resources/*.h",
                "pipelinetasks/*.h"
            ]
            qbs.install: true
            qbs.installDir: engine.INC_PATH + "/engine"
            qbs.installPrefix: engine.PREFIX
        }
    }

    StaticLibrary {
        name: "engine"
        files: engine.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next" }
        bundle.isBundle: false

        cpp.includePaths: engine.incPaths
        cpp.cxxLanguageVersion: engine.languageVersion
        cpp.cxxStandardLibrary: engine.standardLibrary
        cpp.minimumMacosVersion: engine.osxVersion
        cpp.minimumIosVersion: engine.iosVersion
        cpp.minimumTvosVersion: engine.tvosVersion
        cpp.defines: ["ENGINE_LIBRARY", "FT2_BUILD_LIBRARY", "PHYSFS_SUPPORTS_ZIP", "PHYSFS_SUPPORTS_DEFAULT=0", "PHYSFS_NO_CDROM_SUPPORT"]
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: qbs.targetOS.contains("windows") || (qbs.targetOS.contains("linux") && !qbs.targetOS.contains("android"))
            files: outer.concat(["src/adapters/desktopadaptor.cpp"])
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            files: outer.concat(["src/adapters/mobileadaptor.cpp"])
            cpp.defines: outer.concat(["THUNDER_MOBILE"])
            Android.ndk.appStl: engine.ANDROID_STL
            Android.ndk.platform: engine.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            files: outer.concat(["../thirdparty/physfs/src/*.m"])
        }

        Properties {
            condition: qbs.targetOS.contains("darwin") && !(qbs.targetOS.contains("ios") || qbs.targetOS.contains("tvos"))
            files: outer.concat(["src/adapters/mobileadaptor.cpp", "src/adapters/appleplatform.mm"])
            cpp.defines: outer.concat(["THUNDER_MOBILE", "TARGET_OS_OSX"])
        }

        Properties {
            condition: qbs.targetOS.contains("ios")
            files: outer.concat(["src/adapters/mobileadaptor.cpp", "src/adapters/appleplatform.mm"])
            cpp.defines: outer.concat(["THUNDER_MOBILE", "TARGET_OS_IOS"])
        }

        Properties {
            condition: qbs.targetOS.contains("tvos")
            files: outer.concat(["src/adapters/mobileadaptor.cpp", "src/adapters/appleplatform.mm"])
            cpp.defines: outer.concat(["THUNDER_MOBILE", "TARGET_OS_TV"])
        }

        Group {
            name: "Install Static Engine"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: engine.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: engine.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: engine.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: engine.PREFIX
        }
    }
}
