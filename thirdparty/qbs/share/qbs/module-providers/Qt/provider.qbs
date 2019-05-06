import "setup-qt.js" as SetupQt

ModuleProvider {
    property stringList qmakeFilePaths
    relativeSearchPaths: SetupQt.doSetup(qmakeFilePaths, outputBaseDir, path, qbs)
}
