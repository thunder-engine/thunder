import qbs

Project {
    id: resourceCompiler

    CppApplication {
        name: "trc"
        condition: resourceCompiler.desktop
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
