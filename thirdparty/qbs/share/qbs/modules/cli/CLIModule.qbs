// base for Common Language Infrastructure modules
import qbs.FileInfo
import qbs.ModUtils
import "cli.js" as CLI

Module {
    Depends { name: "bundle"; condition: qbs.targetOS.contains("darwin") }
    Properties {
        condition: qbs.targetOS.contains("darwin")
        bundle.isBundle: false
    }

    condition: false

    property string warningLevel: 'all' // 'none', 'all'
    property bool treatWarningsAsErrors: false
    property string architecture: "anycpu" // for the CLI this is a much better default than qbs.architecture
    property string optimization: qbs.optimization
    property bool debugInformation: qbs.debugInformation
    property stringList defines
    property stringList platformDefines: qbs.enableDebugCode ? [] : ["NDEBUG"]
    property stringList compilerDefines
    PropertyOptions {
        name: "compilerDefines"
        description: "preprocessor macros that are defined when using this particular compiler"
    }

    property pathList libraryPaths
    property string csharpCompilerName
    property string csharpCompilerPath: FileInfo.joinPaths(toolchainInstallPath, csharpCompilerName)
    property string vbCompilerName
    property string vbCompilerPath: FileInfo.joinPaths(toolchainInstallPath, vbCompilerName)
    property string fsharpCompilerName
    property string fsharpCompilerPath: FileInfo.joinPaths(toolchainInstallPath, fsharpCompilerName)
    property string resgenName: "resgen"
    property string resgenPath: FileInfo.joinPaths(toolchainInstallPath, resgenName)
    property string dynamicLibrarySuffix: ".dll"
    property string executableSuffix: ".exe"
    property string netmoduleSuffix: ".netmodule"
    property string debugInfoSuffix
    property stringList dynamicLibraries // list of names, will be linked with /reference:name
    property stringList netmodules // list of netmodule files, will be linked with /addmodule:name

    property bool generateManifestFile: true

    property string toolchainInstallPath

    property stringList compilerFlags
    PropertyOptions {
        name: "compilerFlags"
        description: "additional compiler flags"
    }

    // Platform properties. Those are intended to be set by the toolchain setup
    // and are prepended to the corresponding user properties.
    property stringList platformCompilerFlags

    FileTagger {
        patterns: ["*.cs", "*.CS"]
        fileTags: ["cli.csharp"]
    }

    FileTagger {
        patterns: ["*.vb", "*.VB"]
        fileTags: ["cli.vb"]
    }

    FileTagger {
        patterns: ["*.fs", "*.FS"]
        fileTags: ["cli.fsharp"]
    }

    FileTagger {
        patterns: ["*.fsi", "*.FSI"]
        fileTags: ["cli.fsharp_signature"]
    }

    FileTagger {
        patterns: ["*.resx", "*.RESX"]
        fileTags: ["cli.resx"]
    }

    validate: {
        var validator = new ModUtils.PropertyValidator("cli");
        validator.setRequiredProperty("toolchainInstallPath", toolchainInstallPath);
        validator.validate();
    }

    setupBuildEnvironment: {
        var v = new ModUtils.EnvironmentVariable("PATH", product.qbs.pathListSeparator,
                                                 product.qbs.hostOS.contains("windows"));
        v.prepend(product.cli.toolchainInstallPath);
        v.set();
    }

    Rule {
        id: cliApplication
        multiplex: true
        inputs: ["cli.csharp", "cli.vb", "cli.fsharp"]
        inputsFromDependencies: ["cli.netmodule", "dynamiclibrary", "cli.resources"]

        Artifact {
            fileTags: ["application"]
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         product.targetName
                                         + product.moduleProperty(product.moduleName,
                                                                  "executableSuffix"))
        }

        Artifact {
            fileTags: ["debuginfo_app"]
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         product.targetName
                                         + product.moduleProperty(product.moduleName,
                                                                  "debugInfoSuffix"))
        }

        prepare: {
            return CLI.prepareCompiler(product, inputs, outputs.application[0]);
        }
    }

    Rule {
        id: cliDynamicLibrary
        multiplex: true
        inputs: ["cli.csharp", "cli.vb", "cli.fsharp"]
        inputsFromDependencies: ["cli.netmodule", "dynamiclibrary", "cli.resources"]

        Artifact {
            fileTags: ["dynamiclibrary"]
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         product.targetName
                                         + product.moduleProperty(product.moduleName,
                                                                  "dynamicLibrarySuffix"))
        }

        Artifact {
            fileTags: ["debuginfo_dll"]
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         product.targetName
                                         + product.moduleProperty(product.moduleName,
                                                                  "debugInfoSuffix"))
        }

        prepare: {
            return CLI.prepareCompiler(product, inputs, outputs.dynamiclibrary[0]);
        }
    }

    Rule {
        id: netmodule
        multiplex: true
        inputs: ["cli.csharp", "cli.vb", "cli.fsharp"]
        inputsFromDependencies: ["cli.netmodule", "dynamiclibrary", "cli.resources"]

        Artifact {
            fileTags: ["cli.netmodule"]
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         product.targetName
                                         + product.moduleProperty(product.moduleName,
                                                                  "netmoduleSuffix"))
        }

        Artifact {
            fileTags: ["debuginfo_netmodule"]
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         product.targetName
                                         + product.moduleProperty(product.moduleName,
                                                                  "debugInfoSuffix"))
        }

        prepare: {
            return CLI.prepareCompiler(product, inputs, outputs["cli.netmodule"][0]);
        }
    }

    Rule {
        inputs: ["cli.resx"]

        Artifact {
            fileTags: ["cli.resources"]
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         input.completeBaseName + ".resources")
        }

        prepare: {
            var args = [ input.filePath, output.filePath ];
            var cmd = new Command(ModUtils.moduleProperty(product, "resgenPath"), args);
            cmd.description = "building " + input.fileName;
            cmd.highlight = "compiler";
            cmd.workingDirectory = FileInfo.path(output.filePath);
            return cmd;
        }
    }
}
