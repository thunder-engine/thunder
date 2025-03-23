import qbs

Project {
    id: network
    property stringList srcFiles: [
        "src/*.cpp",
        "src/objects/*.cpp",
        "src/utils/*.cpp",
        "includes/utils/*.h",
    ]

    property stringList incPaths: [
        "includes",
        "includes/objects",
        "includes/utils",
        "../../common",
        "../../thirdparty/next/inc",
        "../../thirdparty/next/inc/math",
        "../../thirdparty/next/inc/core",
        "../../thirdparty/ssl/include",
        "../../engine/includes",
        "../../engine/includes/resources",
        "../../engine/includes/editor",
    ]

    DynamicLibrary {
        name: "network-editor"
        condition: network.desktop
        files: srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE", "NETWORK_LIBRARY"]
        cpp.includePaths: network.incPaths
        cpp.staticLibraries: [ "ssl", "crypto" ]
        cpp.cxxLanguageVersion: network.languageVersion
        cpp.cxxStandardLibrary: network.standardLibrary
        cpp.minimumMacosVersion: network.osxVersion

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: outer.concat([
                "Ws2_32", "Crypt32", "Advapi32", "User32"
            ])
            cpp.libraryPaths: [ "../../thirdparty/ssl/lib" ]
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
            cpp.libraryPaths: [ "/opt/homebrew/opt/openssl/lib" ]
        }

        Group {
            name: "Install Dynamic network"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: network.PLUGINS_PATH
            qbs.installPrefix: network.PREFIX
        }

        Group {
            name: "Module includes"
            files: [
                "includes/network.h"
            ]
            qbs.install: true
            qbs.installDir: network.INC_PATH + "/modules"
            qbs.installPrefix: network.PREFIX
        }

        Group {
            name: "Engine includes"
            prefix: "includes/"
            files: [
                "objects/*.h"
            ]
            qbs.install: true
            qbs.installDir: network.INC_PATH + "/engine"
            qbs.installPrefix: network.PREFIX
        }
    }

    StaticLibrary {
        name: "network"
        files: network.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: network.incPaths
        cpp.cxxLanguageVersion: network.languageVersion
        cpp.cxxStandardLibrary: network.standardLibrary
        cpp.minimumMacosVersion: network.osxVersion
        cpp.minimumIosVersion: network.iosVersion
        cpp.minimumTvosVersion: network.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: !network.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: network.ANDROID_STL
            Android.ndk.platform: network.ANDROID
        }

        Group {
            name: "Install Static network"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: network.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: network.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: cpp.debugInformation ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: network.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: network.PREFIX
        }
    }
}
