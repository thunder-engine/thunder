import qbs

Project {
    id: syntaxhighlighting
    property stringList srcFiles: [
        "src/*.cpp",
        "src/*.h",
        "themes/*.qrc",
        "syntax/*.qrc"
    ]

    property stringList incPaths: [
        "src"
    ]

    StaticLibrary {
        name: "syntaxhighlighting"
        condition: syntaxhighlighting.desktop
        files: syntaxhighlighting.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "Qt.core" }
        Depends { name: "Qt.gui" }
        Depends { name: "Qt.network" }
        bundle.isBundle: false

        cpp.defines: [ "HAS_SYNTAX_RESOURCE", "SyntaxHighlighting_VERSION_MAJOR=5", "SyntaxHighlighting_VERSION_MINOR=57" ]
        cpp.includePaths: syntaxhighlighting.incPaths
    }
}
