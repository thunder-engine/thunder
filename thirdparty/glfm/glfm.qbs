import qbs
import qbs.FileInfo

Project {
    id: glfm
    property stringList srcFiles: {
        var sources = [
        "src/glfm_internal.h",
        "include/glfm.h"
        ];
        return sources;
    }

    property stringList incPaths: [
        "include"
    ]

    StaticLibrary {
        name: "glfm"
        condition: !glfm.desktop
        files: glfm.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [ ]
        cpp.includePaths: glfm.incPaths
        cpp.cLanguageVersion: "c11"

        Properties {
            condition: qbs.targetOS.contains("android")
            property string nativePath: FileInfo.joinPaths(Android.ndk.ndkDir, "sources", "android", "native_app_glue")
            files: outer.concat([
                "src/glfm_android.c",
                nativePath + "/android_native_app_glue.c",
                nativePath + "/android_native_app_glue.h"
            ])

            cpp.includePaths: outer.concat([nativePath])
            Android.ndk.appStl: glfm.ANDROID_STL
            Android.ndk.platform: glfm.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("ios") || qbs.targetOS.contains("tvos")
            files: outer.concat(["src/glfm_apple.m"])
        }

        Group {
            name: "Install Static glfm"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: glfm.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: glfm.PREFIX
        }
    }
}
