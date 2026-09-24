import qbs

Project {
    id: engine
    property stringList srcFiles: {
        var sources = [
            "src/*.cpp",
            "src/components/**/*.cpp",
            "src/resources/*.cpp",
            "src/systems/*.cpp",
            "src/filters/*.cpp",
            "src/pipelinetasks/*.cpp",
            "src/utils/*.cpp",
            "src/adapters/platform*.cpp",
        ]

        // Freetype
        sources.push("../thirdparty/freetype/src/base/*tbase.c")
        sources.push("../thirdparty/freetype/src/base/*tinit.c")
        sources.push("../thirdparty/freetype/src/base/*tsystem.c")
        sources.push("../thirdparty/freetype/src/base/*tbbox.c")
        sources.push("../thirdparty/freetype/src/base/*tbdf.c")
        sources.push("../thirdparty/freetype/src/base/*tbitmap.c")
        sources.push("../thirdparty/freetype/src/base/*tcid.c")
        sources.push("../thirdparty/freetype/src/base/*tdebug.c")
        sources.push("../thirdparty/freetype/src/base/*tfstype.c")
        sources.push("../thirdparty/freetype/src/base/*tgasp.c")
        sources.push("../thirdparty/freetype/src/base/*tglyph.c")
        sources.push("../thirdparty/freetype/src/base/*tgxval.c")
        sources.push("../thirdparty/freetype/src/base/*tmm.c")
        sources.push("../thirdparty/freetype/src/base/*totval.c")
        sources.push("../thirdparty/freetype/src/base/*tpatent.c")
        sources.push("../thirdparty/freetype/src/base/*tpfr.c")
        sources.push("../thirdparty/freetype/src/base/*tstroke.c")
        sources.push("../thirdparty/freetype/src/base/*tsynth.c")
        sources.push("../thirdparty/freetype/src/base/*ttype1.c")
        sources.push("../thirdparty/freetype/src/base/*twinfnt.c")
        sources.push("../thirdparty/freetype/src/autofit/*utofit.c")
        sources.push("../thirdparty/freetype/src/bdf/*df.c")
        sources.push("../thirdparty/freetype/src/cff/*ff.c")
        sources.push("../thirdparty/freetype/src/cache/*tcache.c")
        sources.push("../thirdparty/freetype/src/lzw/*tlzw.c")
        sources.push("../thirdparty/freetype/src/pcf/*cf.c")
        sources.push("../thirdparty/freetype/src/pfr/*fr.c")
        sources.push("../thirdparty/freetype/src/psaux/*saux.c")
        sources.push("../thirdparty/freetype/src/pshinter/*shinter.c")
        sources.push("../thirdparty/freetype/src/psnames/*smodule.c")
        sources.push("../thirdparty/freetype/src/raster/raste*.c")
        sources.push("../thirdparty/freetype/src/sdf/sd*.c")
        sources.push("../thirdparty/freetype/src/sfnt/sfn*.c")
        sources.push("../thirdparty/freetype/src/smooth/smoot*.c")
        sources.push("../thirdparty/freetype/src/svg/sv*.c")
        sources.push("../thirdparty/freetype/src/truetype/*ruetype.c")
        sources.push("../thirdparty/freetype/src/type1/*ype1.c")
        sources.push("../thirdparty/freetype/src/cid/*ype1cid.c")
        sources.push("../thirdparty/freetype/src/type42/*ype42.c")
        sources.push("../thirdparty/freetype/src/winfonts/*infnt.c")

        // PhysFS
        sources.push("../thirdparty/physfs/src/*.c")
        if(qbs.targetOS.contains("darwin")) {
            sources.push("../thirdparty/physfs/src/*.m")
        }

        // GLFW
        if(engine.desktop) {
            sources.push("../thirdparty/glfw/src/*ontext.c")
            sources.push("../thirdparty/glfw/src/*nit.c")
            sources.push("../thirdparty/glfw/src/*nput.c")
            sources.push("../thirdparty/glfw/src/*onitor.c")
            sources.push("../thirdparty/glfw/src/*ull_init.c")
            sources.push("../thirdparty/glfw/src/*ull_joystick.c")
            sources.push("../thirdparty/glfw/src/*ull_monitor.c")
            sources.push("../thirdparty/glfw/src/*ull_window.c")
            sources.push("../thirdparty/glfw/src/*latform.c")
            sources.push("../thirdparty/glfw/src/*ulkan.c")
            sources.push("../thirdparty/glfw/src/*indow.c")
            sources.push("../thirdparty/glfw/src/*smesa_context.c")
            sources.push("../thirdparty/glfw/src/*gl_context.c")

            sources.push("../thirdparty/glfw/src/*nternal.h")
            sources.push("../thirdparty/glfw/src/*ull_platform.h")
            sources.push("../thirdparty/glfw/include/GLFW/*lfw3.h")
            sources.push("../thirdparty/glfw/include/GLFW/*lfw3native.h")

            if(qbs.targetOS.contains("windows")) {
                sources.push("../thirdparty/glfw/src/*in32_init.c"),
                sources.push("../thirdparty/glfw/src/*in32_joystick.c"),
                sources.push("../thirdparty/glfw/src/*in32_module.c"),
                sources.push("../thirdparty/glfw/src/*in32_monitor.c"),
                sources.push("../thirdparty/glfw/src/*in32_time.c"),
                sources.push("../thirdparty/glfw/src/*in32_thread.c"),
                sources.push("../thirdparty/glfw/src/*in32_window.c"),
                sources.push("../thirdparty/glfw/src/*gl_context.c"),

                sources.push("../thirdparty/glfw/src/*in32_platform.h"),
                sources.push("../thirdparty/glfw/src/*in32_joystick.h")
            } else if(qbs.targetOS.contains("darwin")) {
                sources.push("../thirdparty/glfw/src/*ocoa_init.m"),
                sources.push("../thirdparty/glfw/src/*ocoa_joystick.m"),
                sources.push("../thirdparty/glfw/src/*ocoa_monitor.m"),
                sources.push("../thirdparty/glfw/src/*ocoa_time.c"),
                sources.push("../thirdparty/glfw/src/*osix_module.c"),
                sources.push("../thirdparty/glfw/src/*osix_poll.c"),
                sources.push("../thirdparty/glfw/src/*osix_thread.c"),
                sources.push("../thirdparty/glfw/src/*ocoa_window.m"),
                sources.push("../thirdparty/glfw/src/*sgl_context.m"),

                sources.push("../thirdparty/glfw/src/*ocoa_platform.h"),
                sources.push("../thirdparty/glfw/src/*ocoa_joystick.h")
            } else if(qbs.targetOS.contains("linux")) {
                sources.push("../thirdparty/glfw/src/*11_init.c"),
                sources.push("../thirdparty/glfw/src/*inux_joystick.c"),
                sources.push("../thirdparty/glfw/src/*kb_unicode.c"),
                sources.push("../thirdparty/glfw/src/*11_monitor.c"),
                sources.push("../thirdparty/glfw/src/*osix_module.c"),
                sources.push("../thirdparty/glfw/src/*osix_poll.c"),
                sources.push("../thirdparty/glfw/src/*osix_time.c"),
                sources.push("../thirdparty/glfw/src/*osix_thread.c"),
                sources.push("../thirdparty/glfw/src/*11_window.c"),
                sources.push("../thirdparty/glfw/src/*lx_context.c"),

                sources.push("../thirdparty/glfw/src/*11_platform.h"),
                sources.push("../thirdparty/glfw/src/*inux_joystick.h")
            }
        }

        return sources;
    }

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
        files: engine.srcFiles

        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
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
            cpp.defines: outer.concat(["_GLFW_WIN32"])
            cpp.dynamicLibraries: outer.concat(["Shell32", "Advapi32", "Gdi32", "User32"])
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.defines: outer.concat(["_GLFW_X11"])
            cpp.rpaths: "$ORIGIN/../lib"
            cpp.dynamicLibraries: outer.concat(["X11", "Xrandr", "Xi", "Xxf86vm", "Xcursor", "Xinerama"])
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["IOKit", "Foundation", "CoreFoundation", "AppKit", "CoreVideo"]
            cpp.defines: outer.concat(["PHYSFS_DARWIN", "_GLFW_COCOA"])
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
            condition: qbs.targetOS.contains("windows")
            cpp.defines: outer.concat(["_GLFW_WIN32"])
        }

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
