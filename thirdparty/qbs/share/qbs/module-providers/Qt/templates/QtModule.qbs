import qbs.FileInfo

Module {
    condition: (qbs.targetPlatform === targetPlatform || isCombinedUIKitBuild)
               && (!qbs.architecture
                   || architectures.length === 0
                   || architectures.contains(qbs.architecture))

    readonly property bool isCombinedUIKitBuild: ["ios", "tvos", "watchos"].contains(targetPlatform)
        && ["x86", "x86_64"].contains(qbs.architecture)
        && qbs.targetPlatform === targetPlatform + "-simulator"

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }

    Depends { name: "Qt.plugin_support" }
    property stringList pluginTypes
    Qt.plugin_support.pluginTypes: pluginTypes
    Depends {
        condition: Qt.core.staticBuild && !isPlugin
        name: "Qt";
        submodules: {
            // We have to pull in all plugins here, because dependency resolving happens
            // before module merging, and we don't know yet if someone set
            // Qt.pluginSupport.pluginsByType in the product.
            // The real filtering is done later by the plugin module files themselves.
            var list = [];
            var allPlugins = Qt.plugin_support.allPluginsByType;
            for (var i = 0; i < (pluginTypes || []).length; ++i)
                Array.prototype.push.apply(list, allPlugins[pluginTypes[i]])
            return list;
        }
    }

    property string qtModuleName
    property path binPath: Qt.core.binPath
    property path incPath: Qt.core.incPath
    property path libPath: Qt.core.libPath
    property string qtLibInfix: Qt.core.libInfix
    property string libNameForLinkerDebug
    property string libNameForLinkerRelease
    property string libNameForLinker: Qt.core.qtBuildVariant === "debug"
                                      ? libNameForLinkerDebug : libNameForLinkerRelease
    property string libFilePathDebug
    property string libFilePathRelease
    property string libFilePath: Qt.core.qtBuildVariant === "debug"
                                 ? libFilePathDebug : libFilePathRelease
    version: Qt.core.version
    property bool hasLibrary: true
    property bool isStaticLibrary: false
    property bool isPlugin: false

    property stringList architectures
    property string targetPlatform
    property stringList staticLibsDebug
    property stringList staticLibsRelease
    property stringList dynamicLibsDebug
    property stringList dynamicLibsRelease
    property stringList linkerFlagsDebug
    property stringList linkerFlagsRelease
    property stringList staticLibs: Qt.core.qtBuildVariant === "debug"
                                    ? staticLibsDebug : staticLibsRelease
    property stringList dynamicLibs: Qt.core.qtBuildVariant === "debug"
                                    ? dynamicLibsDebug : dynamicLibsRelease
    property stringList frameworksDebug
    property stringList frameworksRelease
    property stringList frameworkPathsDebug
    property stringList frameworkPathsRelease
    property stringList mFrameworks: Qt.core.qtBuildVariant === "debug"
            ? frameworksDebug : frameworksRelease
    property stringList mFrameworkPaths: Qt.core.qtBuildVariant === "debug"
            ? frameworkPathsDebug: frameworkPathsRelease
    cpp.linkerFlags: Qt.core.qtBuildVariant === "debug"
            ? linkerFlagsDebug : linkerFlagsRelease
    property bool enableLinking: qtModuleName != undefined && hasLibrary
    property stringList moduleConfig

    Properties {
        condition: enableLinking
        cpp.staticLibraries: staticLibs
        cpp.dynamicLibraries: dynamicLibs
        cpp.frameworks: mFrameworks.concat(!isStaticLibrary && Qt.core.frameworkBuild
                        ? [libNameForLinker] : [])
        cpp.frameworkPaths: mFrameworkPaths
    }
}
