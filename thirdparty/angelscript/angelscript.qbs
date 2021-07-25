import qbs

Project {
    id: angelscript
    property stringList srcFiles: [
        "source/*.cpp"
    ]

    property stringList incPaths: [
        "include"
    ]

    DynamicLibrary {
        name: "angelscript-editor"
        condition: angelscript.desktop
        files: angelscript.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [ "ANGELSCRIPT_EXPORT" ]
        cpp.includePaths: angelscript.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: "libc++"
        cpp.minimumMacosVersion: "10.12"

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Properties {
            condition: qbs.architecture === "x86_64" && qbs.targetOS.contains("windows")
            files: outer.concat(["source/as_callfunc_x64_msvc_asm.asm"])
        }

        Group {
            name: "Install Dynamic Platform"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: angelscript.LIB_PATH + "/" + angelscript.bundle
            qbs.installPrefix: angelscript.PREFIX
        }

    }

    StaticLibrary {
        name: "angelscript"
        files: angelscript.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [ "ANGELSCRIPT_EXPORT", "AS_NO_COMPILER" ]
        cpp.includePaths: angelscript.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: "libc++"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"

        Properties {
            condition: qbs.architecture === "x86_64" && qbs.targetOS.contains("windows")
            files: outer.concat(["source/as_callfunc_x64_msvc_asm.asm"])
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: angelscript.ANDROID_STL
            Android.ndk.platform: angelscript.ANDROID
        }

        Group {
            name: "Install Static angelscript"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir:  angelscript.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: angelscript.PREFIX
        }
    }
}
