import qbs

Project {
    id: next
    property stringList srcFiles: [
        "src/*.cpp",
        "src/core/*.cpp",
        "src/math/*.cpp",
        "src/anim/*.cpp",
        "src/analytics/*.cpp",
        "src/network/*.cpp"
    ]

    property stringList incPaths: {
            var sources = [
                "inc",
                "inc/core",
                "inc/math",
                "inc/anim",
                "inc/analytics",
                "inc/network",
                "inc/os",
                "../ssl/include"
            ];
            if(qbs.targetOS.contains("linux")) {
                sources.push("/usr/include/gtk-3.0")
                sources.push("/usr/include/glib-2.0")
                sources.push("/usr/include/pango-1.0")
                sources.push("/usr/include/atk-1.0")
                sources.push("/usr/include/gdk-pixbuf-2.0")
                sources.push("/usr/include/harfbuzz")
                sources.push("/usr/include/cairo")
                sources.push("/usr/lib/x86_64-linux-gnu/glib-2.0/include")
            }

            return sources;
        }

    DynamicLibrary {
        name: "next-editor"
        condition: next.desktop
        files: {
            var sources = srcFiles
            sources.push("src/os/*.cpp")
            return sources
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE", "NEXT_LIBRARY"]
        cpp.includePaths: next.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ "ssl", "crypto" ]
        cpp.cxxLanguageVersion: next.languageVersion
        cpp.cxxStandardLibrary: next.standardLibrary
        cpp.minimumMacosVersion: next.osxVersion

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: outer.concat([
                "Shell32", "Ws2_32", "Crypt32", "Advapi32", "User32"
            ])
            cpp.libraryPaths: [ "../../thirdparty/ssl/lib" ]
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
            cpp.dynamicLibraries: outer.concat(["objc"])
            cpp.libraryPaths: [ "/opt/homebrew/opt/openssl/lib" ]
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.dynamicLibraries: outer.concat(["gtk-3"])
        }

        Group {
            name: "Install Dynamic Platform"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: next.LIB_PATH
            qbs.installPrefix: next.PREFIX
        }

        Group {
            name: "Next includes"
            prefix: "inc/"
            files: [
                "**"
            ]
            qbs.install: true
            qbs.installDir: next.INC_PATH + "/next"
            qbs.installPrefix: next.PREFIX
            qbs.installSourceBase: prefix
        }
    }

    StaticLibrary {
        name: "next"
        files: next.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["NEXT_LIBRARY"]
        cpp.includePaths: next.incPaths
        cpp.cxxLanguageVersion: next.languageVersion
        cpp.cxxStandardLibrary: next.standardLibrary
        cpp.minimumMacosVersion: next.osxVersion
        cpp.minimumIosVersion: next.iosVersion
        cpp.minimumTvosVersion: next.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: next.desktop
            files: outer.concat([
                "src/os/*.cpp"
            ])
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: next.ANDROID_STL
            Android.ndk.platform: next.ANDROID
        }

        Group {
            name: "Install Static Platform"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: next.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: next.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: next.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: next.PREFIX
        }
    }
}
