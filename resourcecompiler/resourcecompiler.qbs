import qbs

Project {
    id: resourceCompiler

    CppApplication {
        name: "trc"
        condition: qbs.targetOS.contains("windows") ||
                   qbs.targetOS.contains("linux") ||
                   qbs.targetOS.contains("darwin")

        files: ["main.cpp"]

        Depends { name: "pugixml" }

        cpp.includePaths: ["../thirdparty/pugixml/src"]
        cpp.cxxLanguageVersion: "c++17"

        Group {
            name: "Install Resource Compiler"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: resourceCompiler.TOOLS_PATH
            qbs.installPrefix: resourceCompiler.PREFIX
        }
    }
}
