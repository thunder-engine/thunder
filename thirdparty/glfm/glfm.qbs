import qbs
import qbs.FileInfo

Project {
    id: glfm
    property stringList srcFiles: {
        var sources = [
        "src/glfm_platform.h",
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

        cpp.defines: [ ]
        cpp.includePaths: glfm.incPaths
        cpp.cLanguageVersion: "c11"

        Properties {
            condition: qbs.targetOS.contains("android")
            property string nativePath: FileInfo.joinPaths(Android.ndk.ndkDir, "sources", "android", "native_app_glue")
            files: outer.concat([
                "src/glfm_platform_android.c",
                nativePath + "/android_native_app_glue.c",
                nativePath + "/android_native_app_glue.h"
            ])

            cpp.includePaths: outer.concat([nativePath])
            Android.ndk.appStl: "gnustl_shared"
        }

        Properties {
            condition: qbs.targetOS.contains("ios")
            files: outer.concat(["src/glfm_platform_ios.m"])
        }

        Group {
            name: "Install Static glfm"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: glfm.LIB_PATH
            qbs.installPrefix: glfm.PREFIX
        }
    }
}
