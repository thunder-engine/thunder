import qbs

Project {
    id: glad
    property stringList srcFiles: [
        "src/*.c"
    ]

    property stringList incPaths: [
        "include"
    ]

    StaticLibrary {
        name: "glad"
        condition: glad.desktop
        files: glad.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [  ]
        cpp.includePaths: glad.incPaths

        Group {
            name: "Install Static GLAD"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: glad.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: glad.PREFIX
        }
    }
}
