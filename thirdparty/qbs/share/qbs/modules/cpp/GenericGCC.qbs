/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qbs.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.PathTools
import qbs.Probes
import qbs.Process
import qbs.TextFile
import qbs.Utilities
import qbs.UnixUtils
import qbs.WindowsUtils
import 'gcc.js' as Gcc

CppModule {
    condition: qbs.toolchain && qbs.toolchain.contains("gcc")
    priority: -100

    Probes.GccBinaryProbe {
        id: compilerPathProbe
        condition: !toolchainInstallPath && !_skipAllChecks
        _compilerName: compilerName
        _toolchainPrefix: toolchainPrefix
    }

    // Find the version as early as possible in case other things depend on it,
    // for example the question of whether certain flags are supported and which need to be used
    // in the GccProbe itself.
    Probes.GccVersionProbe {
        id: gccVersionProbe
        compilerFilePath: compilerPath
        environment: probeEnv
    }

    Probes.GccProbe {
        id: gccProbe
        condition: !_skipAllChecks
        compilerFilePathByLanguage: compilerPathByLanguage
        enableDefinesByLanguage: enableCompilerDefinesByLanguage
        environment: probeEnv
        flags: targetDriverFlags.concat(sysrootFlags)
        _sysroot: sysroot
    }
    property var probeEnv

    Probes.BinaryProbe {
        id: binutilsProbe
        condition: !File.exists(archiverPath) && !_skipAllChecks
        names: Gcc.toolNames([archiverName, assemblerName, linkerName, nmName,
                              objcopyName, stripName], toolchainPrefix)
    }

    targetLinkerFlags: Gcc.targetFlags("linker", false,
                                       target, targetArch, machineType, qbs.targetOS)
    targetAssemblerFlags: Gcc.targetFlags("assembler", assemblerHasTargetOption,
                                          target, targetArch, machineType, qbs.targetOS)
    targetDriverFlags: Gcc.targetFlags("compiler", compilerHasTargetOption,
                                       target, targetArch, machineType, qbs.targetOS)

    Probe {
        id: nmProbe
        condition: !_skipAllChecks
        property string theNmPath: nmPath
        property bool hasDynamicOption
        configure: {
            var proc = new Process();
            try {
                hasDynamicOption = proc.exec(theNmPath, ["-D", theNmPath], false) == 0;
                console.debug("nm has -D: " + hasDynamicOption);
            } finally {
                proc.close();
            }
        }
    }

    qbs.architecture: gccProbe.found ? gccProbe.architecture : original
    endianness: gccProbe.endianness

    compilerDefinesByLanguage: gccProbe.compilerDefinesByLanguage

    compilerVersionMajor: gccVersionProbe.versionMajor
    compilerVersionMinor: gccVersionProbe.versionMinor
    compilerVersionPatch: gccVersionProbe.versionPatch

    compilerIncludePaths: gccProbe.includePaths
    compilerFrameworkPaths: gccProbe.frameworkPaths
    compilerLibraryPaths: gccProbe.libraryPaths

    property bool compilerHasTargetOption: qbs.toolchain.contains("clang")
                                           && Utilities.versionCompare(compilerVersion, "3.1") >= 0
    property bool assemblerHasTargetOption: qbs.toolchain.contains("xcode")
                                            && Utilities.versionCompare(compilerVersion, "7") >= 0
    property string target: targetArch
                            ? [targetArch, targetVendor, targetSystem, targetAbi].join("-")
                            : undefined
    property string targetArch: Utilities.canonicalTargetArchitecture(
                                    qbs.architecture, endianness,
                                    targetVendor, targetSystem, targetAbi)
    property string targetVendor: "unknown"
    property string targetSystem: "unknown"
    property string targetAbi: "unknown"

    property string toolchainPrefix: compilerPathProbe.found
                                     ? compilerPathProbe.tcPrefix
                                     : undefined
    property string toolchainInstallPath: compilerPathProbe.found ? compilerPathProbe.path
                                                                  : undefined
    property string binutilsPath: binutilsProbe.found ? binutilsProbe.path : toolchainInstallPath

    assemblerName: 'as'
    compilerName: cxxCompilerName
    linkerName: 'ld'
    property string archiverName: 'ar'
    property string nmName: 'nm'
    property string objcopyName: "objcopy"
    property string stripName: "strip"
    property string dsymutilName: "dsymutil"
    property string lipoName
    property string sysroot: qbs.sysroot
    property string syslibroot: sysroot
    property stringList sysrootFlags: sysroot ? ["--sysroot=" + sysroot] : []

    property string linkerMode: "automatic"
    PropertyOptions {
        name: "linkerMode"
        allowedValues: ["automatic", "manual"]
        description: "Controls whether to automatically use an appropriate compiler frontend "
            + "in place of the system linker when linking binaries. The default is \"automatic\", "
            + "which chooses either the C++ compiler, C compiler, or system linker specified by "
            + "the linkerName/linkerPath properties, depending on the type of object files "
            + "present on the linker command line. \"manual\" allows you to explicitly specify "
            + "the linker using the linkerName/linkerPath properties, and allows linker flags "
            + "passed to the linkerFlags and platformLinkerFlags properties to be escaped "
            + "manually (using -Wl or -Xlinker) instead of automatically based on the selected "
            + "linker."
    }

    property string exportedSymbolsCheckMode: "ignore-undefined"
    PropertyOptions {
        name: "exportedSymbolsCheckMode"
        allowedValues: ["strict", "ignore-undefined"]
        description: "Controls when we consider an updated dynamic library as changed with "
            + "regards to other binaries depending on it. The default is \"ignore-undefined\", "
            + "which means we do not care about undefined symbols being added or removed. "
            + "If you do care about that, e.g. because you link dependent products with an option "
            + "such as \"--no-undefined\", then you should set this property to \"strict\"."
    }

    property string linkerVariant
    PropertyOptions {
        name: "linkerVariant"
        allowedValues: ["bfd", "gold", "lld"]
        description: "Allows to specify the linker variant. Maps to gcc's and clang's -fuse-ld "
                     + "option."
    }
    Properties {
        condition: linkerVariant
        driverLinkerFlags: "-fuse-ld=" + linkerVariant
    }

    property string toolchainPathPrefix: Gcc.pathPrefix(toolchainInstallPath, toolchainPrefix)
    property string binutilsPathPrefix: Gcc.pathPrefix(binutilsPath, toolchainPrefix)

    property string compilerExtension: qbs.hostOS.contains("windows") ? ".exe" : ""
    property string cCompilerName: (qbs.toolchain.contains("clang") ? "clang" : "gcc")
                                   + compilerExtension
    property string cxxCompilerName: (qbs.toolchain.contains("clang") ? "clang++" : "g++")
                                     + compilerExtension

    compilerPathByLanguage: ({
        "c": toolchainPathPrefix + cCompilerName,
        "cpp": toolchainPathPrefix + cxxCompilerName,
        "objc": toolchainPathPrefix + cCompilerName,
        "objcpp": toolchainPathPrefix + cxxCompilerName,
        "asm_cpp": toolchainPathPrefix + cCompilerName
    })

    assemblerPath: binutilsPathPrefix + assemblerName
    compilerPath: toolchainPathPrefix + compilerName
    linkerPath: binutilsPathPrefix + linkerName
    property string archiverPath: binutilsPathPrefix + archiverName
    property string nmPath: binutilsPathPrefix + nmName
    property bool _nmHasDynamicOption: nmProbe.hasDynamicOption
    property string objcopyPath: binutilsPathPrefix + objcopyName
    property string stripPath: binutilsPathPrefix + stripName
    property string dsymutilPath: toolchainPathPrefix + dsymutilName
    property string lipoPath
    property stringList dsymutilFlags

    property bool alwaysUseLipo: false
    property string includeFlag: "-I"
    property string systemIncludeFlag: "-isystem"

    readonly property bool shouldCreateSymlinks: {
        return createSymlinks && internalVersion && ["macho", "elf"].contains(cpp.imageFormat);
    }

    property string internalVersion: {
        if (product.version === undefined)
            return undefined;

        if (!Gcc.isNumericProductVersion(product.version)) {
            // Dynamic library version numbers like "A" or "B" are common on Apple platforms, so
            // don't restrict the product version to a componentized version number here.
            if (cpp.imageFormat === "macho")
                return product.version;

            throw("product.version must be a string in the format x[.y[.z[.w]] "
                  + "where each component is an integer");
        }

        var maxVersionParts = 3;
        var versionParts = product.version.split('.').slice(0, maxVersionParts);

        // pad if necessary
        for (var i = versionParts.length; i < maxVersionParts; ++i)
            versionParts.push("0");

        return versionParts.join('.');
    }
    property string soVersion: {
        var v = internalVersion;
        if (!Gcc.isNumericProductVersion(v))
            return "";
        return v.split('.')[0];
    }

    property var buildEnv: {
        var env = {};
        if (qbs.toolchain.contains("mingw"))
            env.PATH = [toolchainInstallPath]; // For libwinpthread etc
        return env;
    }

    exceptionHandlingModel: {
        if (qbs.toolchain.contains("mingw")) {
            // https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html claims
            // __USING_SJLJ_EXCEPTIONS__ is defined as 1 when using SJLJ exceptions, but there don't
            // seem to be defines for the other models, so use the presence of the DLLs for now.
            var prefix = toolchainInstallPath;
            if (!qbs.hostOS.contains("windows"))
                prefix = FileInfo.joinPaths(toolchainInstallPath, "..", "lib", "gcc",
                                            toolchainPrefix,
                                            [compilerVersionMajor, compilerVersionMinor].join("."));
            var models = ["seh", "sjlj", "dw2"];
            for (var i = 0; i < models.length; ++i) {
                if (File.exists(FileInfo.joinPaths(prefix, "libgcc_s_" + models[i] + "-1.dll"))) {
                    return models[i];
                }
            }
        }
        return base;
    }

    validate: {
        if (_skipAllChecks)
            return;
        if (!File.exists(compilerPath)) {
            var pathMessage = FileInfo.isAbsolutePath(compilerPath)
                    ? "at '" + compilerPath + "'"
                    : "'" + compilerPath + "' in PATH";
            throw ModUtils.ModuleError("Could not find selected C++ compiler " + pathMessage + ". "
                                       + "Ensure that the compiler is properly "
                                       + "installed, or set cpp.toolchainInstallPath to a valid "
                                       + "toolchain path, or consider whether you meant to set "
                                       + "cpp.compilerName instead.");
        }

        var isWrongTriple = false;

        if (gccProbe.architecture) {
            if (Utilities.canonicalArchitecture(architecture)
                    !== Utilities.canonicalArchitecture(gccProbe.architecture))
                isWrongTriple = true;
        } else if (architecture) {
            // This is a warning and not an error on the rare chance some new architecture comes
            // about which qbs does not know about the macros of. But it *might* still work.
            console.warn("Unknown architecture '" + architecture + "' " +
                         "may not be supported by this compiler.");
        }

        if (gccProbe.endianness) {
            if (endianness !== gccProbe.endianness)
                isWrongTriple = true;
        } else if (endianness) {
            console.warn("Could not detect endianness ('"
                         + endianness + "' given)");
        }

        if (gccProbe.targetPlatform) {
            // Can't differentiate Darwin OSes at the compiler level alone
            if (gccProbe.targetPlatform === "darwin"
                    ? !qbs.targetOS.contains("darwin")
                    : qbs.targetPlatform !== gccProbe.targetPlatform)
                isWrongTriple = true;
        } else if (qbs.targetPlatform) {
            console.warn("Could not detect target platform ('"
                         + qbs.targetPlatform + "' given)");
        }

        if (isWrongTriple) {
            var realTriple = [
                Utilities.canonicalArchitecture(gccProbe.architecture),
                gccProbe.targetPlatform,
                gccProbe.endianness ? gccProbe.endianness + "-endian" : undefined
            ].join("-");
            var givenTriple = [
                Utilities.canonicalArchitecture(architecture),
                qbs.targetPlatform,
                endianness ? endianness + "-endian" : undefined
            ].join("-");
            var msg = "The selected compiler '" + compilerPath + "' produces code for '" +
                    realTriple + "', but '" + givenTriple + "' was given, which is incompatible.";
            if (validateTargetTriple) {
                msg +=  " If you are absolutely certain that your configuration is correct " +
                        "(check the values of the qbs.architecture, qbs.targetPlatform, " +
                        "and qbs.endianness properties) and that this message has been " +
                        "emitted in error, set the cpp.validateTargetTriple property to " +
                        "false. However, you should consider submitting a bug report in any " +
                        "case.";
                throw ModUtils.ModuleError(msg);
            } else {
                console.warn(msg);
            }
        }

        var validateFlagsFunction = function (value) {
            if (value) {
                for (var i = 0; i < value.length; ++i) {
                    if (["-target", "-triple", "-arch"].contains(value[i]))
                        return false;
                }
            }
            return true;
        }

        var validator = new ModUtils.PropertyValidator("cpp");
        var msg = "'-target', '-triple' and '-arch' cannot appear in flags; set qbs.architecture instead";
        validator.addCustomValidator("assemblerFlags", assemblerFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("cppFlags", cppFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("cFlags", cFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("cxxFlags", cxxFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("objcFlags", objcFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("objcxxFlags", objcxxFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("commonCompilerFlags", commonCompilerFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("platformAssemblerFlags", platformAssemblerFlags, validateFlagsFunction, msg);
        //validator.addCustomValidator("platformCppFlags", platformCppFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("platformCFlags", platformCFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("platformCxxFlags", platformCxxFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("platformObjcFlags", platformObjcFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("platformObjcxxFlags", platformObjcxxFlags, validateFlagsFunction, msg);
        validator.addCustomValidator("platformCommonCompilerFlags", platformCommonCompilerFlags, validateFlagsFunction, msg);

        validator.setRequiredProperty("compilerVersion", compilerVersion);
        validator.setRequiredProperty("compilerVersionMajor", compilerVersionMajor);
        validator.setRequiredProperty("compilerVersionMinor", compilerVersionMinor);
        validator.setRequiredProperty("compilerVersionPatch", compilerVersionPatch);
        validator.addVersionValidator("compilerVersion", compilerVersion, 3, 3);
        validator.addRangeValidator("compilerVersionMajor", compilerVersionMajor, 1);
        validator.addRangeValidator("compilerVersionMinor", compilerVersionMinor, 0);
        validator.addRangeValidator("compilerVersionPatch", compilerVersionPatch, 0);

        validator.setRequiredProperty("compilerIncludePaths", compilerIncludePaths);
        validator.setRequiredProperty("compilerFrameworkPaths", compilerFrameworkPaths);
        validator.setRequiredProperty("compilerLibraryPaths", compilerLibraryPaths);

        validator.validate();
    }

    // Product should be linked if it's not multiplexed or aggregated at all,
    // or if it is multiplexed, if it's not the aggregate product
    readonly property bool shouldLink: !(product.multiplexed || product.aggregate)
                                       || product.multiplexConfigurationId

    Rule {
        name: "dynamicLibraryLinker"
        condition: product.cpp.shouldLink
        multiplex: true
        inputs: {
            var tags = ["obj", "linkerscript", "versionscript"];
            if (product.bundle && product.bundle.embedInfoPlist
                    && product.qbs.targetOS.contains("darwin")) {
                tags.push("aggregate_infoplist");
            }
            return tags;
        }
        inputsFromDependencies: ["dynamiclibrary_symbols", "staticlibrary", "dynamiclibrary_import"]

        outputFileTags: [
            "bundle.input",
            "dynamiclibrary", "dynamiclibrary_symlink", "dynamiclibrary_symbols", "debuginfo_dll",
            "debuginfo_bundle","dynamiclibrary_import", "debuginfo_plist",
        ]
        outputArtifacts: {
            var artifacts = [{
                filePath: product.destinationDirectory + "/"
                          + PathTools.dynamicLibraryFilePath(product),
                fileTags: ["bundle.input", "dynamiclibrary"],
                bundle: {
                    _bundleFilePath: product.destinationDirectory + "/"
                                     + PathTools.bundleExecutableFilePath(product)
                }
            }];
            if (product.qbs.toolchain.contains("mingw")) {
                artifacts.push({
                    fileTags: ["dynamiclibrary_import"],
                    filePath: FileInfo.joinPaths(product.destinationDirectory,
                                                 PathTools.importLibraryFilePath(product)),
                    alwaysUpdated: false
                });
            } else {
                // List of libfoo's public symbols for smart re-linking.
                artifacts.push({
                    filePath: product.destinationDirectory + "/.sosymbols/"
                              + PathTools.dynamicLibraryFilePath(product),
                    fileTags: ["dynamiclibrary_symbols"],
                    alwaysUpdated: false,
                });
            }

            if (product.cpp.shouldCreateSymlinks && (!product.bundle || !product.bundle.isBundle)) {
                var maxVersionParts = Gcc.isNumericProductVersion(product.version) ? 3 : 1;
                for (var i = 0; i < maxVersionParts; ++i) {
                    var symlink = {
                        filePath: product.destinationDirectory + "/"
                                  + PathTools.dynamicLibraryFilePath(product, undefined, undefined,
                                                                     i),
                        fileTags: ["dynamiclibrary_symlink"]
                    };
                    if (i > 0 && artifacts[i-1].filePath == symlink.filePath)
                        break; // Version number has less than three components.
                    artifacts.push(symlink);
                }
            }
            if (!product.aggregate)
                artifacts = artifacts.concat(Gcc.debugInfoArtifacts(product, undefined, "dll"));
            return artifacts;
        }

        prepare: {
            return Gcc.prepareLinker.apply(Gcc, arguments);
        }
    }

    Rule {
        name: "staticLibraryLinker"
        condition: product.cpp.shouldLink
        multiplex: true
        inputs: ["obj", "linkerscript"]
        inputsFromDependencies: ["dynamiclibrary_symbols", "dynamiclibrary_import", "staticlibrary"]

        outputFileTags: ["bundle.input", "staticlibrary", "c_staticlibrary", "cpp_staticlibrary"]
        outputArtifacts: {
            var tags = ["bundle.input", "staticlibrary"];
            var objs = inputs["obj"];
            var objCount = objs ? objs.length : 0;
            for (var i = 0; i < objCount; ++i) {
                var ft = objs[i].fileTags;
                if (ft.contains("c_obj"))
                    tags.push("c_staticlibrary");
                if (ft.contains("cpp_obj"))
                    tags.push("cpp_staticlibrary");
            }
            return [{
                filePath: FileInfo.joinPaths(product.destinationDirectory,
                                             PathTools.staticLibraryFilePath(product)),
                fileTags: tags,
                bundle: {
                    _bundleFilePath: FileInfo.joinPaths(product.destinationDirectory,
                                                        PathTools.bundleExecutableFilePath(product))
                }
            }];
        }

        prepare: {
            var args = ['rcs', output.filePath];
            for (var i in inputs.obj)
                args.push(inputs.obj[i].filePath);
            var cmd = new Command(product.cpp.archiverPath, args);
            cmd.description = 'creating ' + output.fileName;
            cmd.highlight = 'linker'
            cmd.jobPool = "linker";
            cmd.responseFileUsagePrefix = '@';
            return cmd;
        }
    }

    Rule {
        name: "loadableModuleLinker"
        condition: product.cpp.shouldLink
        multiplex: true
        inputs: {
            var tags = ["obj", "linkerscript"];
            if (product.bundle && product.bundle.embedInfoPlist
                    && product.qbs.targetOS.contains("darwin")) {
                tags.push("aggregate_infoplist");
            }
            return tags;
        }
        inputsFromDependencies: ["dynamiclibrary_symbols", "dynamiclibrary_import", "staticlibrary"]

        outputFileTags: ["bundle.input", "loadablemodule", "debuginfo_loadablemodule",
                         "debuginfo_bundle","debuginfo_plist"]
        outputArtifacts: {
            var app = {
                filePath: FileInfo.joinPaths(product.destinationDirectory,
                                             PathTools.loadableModuleFilePath(product)),
                fileTags: ["bundle.input", "loadablemodule"],
                bundle: {
                    _bundleFilePath: FileInfo.joinPaths(product.destinationDirectory,
                                                        PathTools.bundleExecutableFilePath(product))
                }
            }
            var artifacts = [app];
            if (!product.aggregate)
                artifacts = artifacts.concat(Gcc.debugInfoArtifacts(product, undefined,
                                                                    "loadablemodule"));
            return artifacts;
        }

        prepare: {
            return Gcc.prepareLinker.apply(Gcc, arguments);
        }
    }

    Rule {
        name: "applicationLinker"
        condition: product.cpp.shouldLink
        multiplex: true
        inputs: {
            var tags = ["obj", "linkerscript"];
            if (product.bundle && product.bundle.embedInfoPlist
                    && product.qbs.targetOS.contains("darwin")) {
                tags.push("aggregate_infoplist");
            }
            return tags;
        }
        inputsFromDependencies: ["dynamiclibrary_symbols", "dynamiclibrary_import", "staticlibrary"]

        outputFileTags: ["bundle.input", "application", "debuginfo_app","debuginfo_bundle",
                         "debuginfo_plist"]
        outputArtifacts: {
            var app = {
                filePath: FileInfo.joinPaths(product.destinationDirectory,
                                             PathTools.applicationFilePath(product)),
                fileTags: ["bundle.input", "application"],
                bundle: {
                    _bundleFilePath: FileInfo.joinPaths(product.destinationDirectory,
                                                        PathTools.bundleExecutableFilePath(product))
                }
            }
            var artifacts = [app];
            if (!product.aggregate)
                artifacts = artifacts.concat(Gcc.debugInfoArtifacts(product, undefined, "app"));
            return artifacts;
        }

        prepare: {
            return Gcc.prepareLinker.apply(Gcc, arguments);
        }
    }

    Rule {
        name: "compiler"
        inputs: ["cpp", "c", "objcpp", "objc", "asm_cpp"]
        auxiliaryInputs: ["hpp"]
        explicitlyDependsOn: ["c_pch", "cpp_pch", "objc_pch", "objcpp_pch"]

        outputFileTags: ["obj", "c_obj", "cpp_obj", "intermediate_obj"]
        outputArtifacts: {
            var tags;
            if (input.fileTags.contains("cpp_intermediate_object"))
                tags = ["intermediate_obj"];
            else
                tags = ["obj"];
            if (inputs.c || inputs.objc)
                tags.push("c_obj");
            if (inputs.cpp || inputs.objcpp)
                tags.push("cpp_obj");
            return [{
                fileTags: tags,
                filePath: FileInfo.joinPaths(Utilities.getHash(input.baseDir),
                                             input.fileName + ".o")
            }];
        }

        prepare: {
            return Gcc.prepareCompiler.apply(Gcc, arguments);
        }
    }

    Rule {
        name: "assembler"
        inputs: ["asm"]

        Artifact {
            fileTags: ["obj"]
            filePath: FileInfo.joinPaths(Utilities.getHash(input.baseDir), input.fileName + ".o")
        }

        prepare: {
            return Gcc.prepareAssembler.apply(Gcc, arguments);
        }
    }

    Rule {
        condition: useCPrecompiledHeader
        inputs: ["c_pch_src"]
        auxiliaryInputs: ["hpp"]
        Artifact {
            filePath: product.name + "_c.gch"
            fileTags: ["c_pch"]
        }
        prepare: {
            return Gcc.prepareCompiler.apply(Gcc, arguments);
        }
    }

    Rule {
        condition: useCxxPrecompiledHeader
        inputs: ["cpp_pch_src"]
        auxiliaryInputs: ["hpp"]
        Artifact {
            filePath: product.name + "_cpp.gch"
            fileTags: ["cpp_pch"]
        }
        prepare: {
            return Gcc.prepareCompiler.apply(Gcc, arguments);
        }
    }

    Rule {
        condition: useObjcPrecompiledHeader
        inputs: ["objc_pch_src"]
        auxiliaryInputs: ["hpp"]
        Artifact {
            filePath: product.name + "_objc.gch"
            fileTags: ["objc_pch"]
        }
        prepare: {
            return Gcc.prepareCompiler.apply(Gcc, arguments);
        }
    }

    Rule {
        condition: useObjcxxPrecompiledHeader
        inputs: ["objcpp_pch_src"]
        auxiliaryInputs: ["hpp"]
        Artifact {
            filePath: product.name + "_objcpp.gch"
            fileTags: ["objcpp_pch"]
        }
        prepare: {
            return Gcc.prepareCompiler.apply(Gcc, arguments);
        }
    }

    FileTagger {
        patterns: "*.s"
        fileTags: ["asm"]
    }

    FileTagger {
        patterns: "*.S"
        fileTags: ["asm_cpp"]
    }

    FileTagger {
        patterns: "*.sx"
        fileTags: ["asm_cpp"]
    }

    Scanner {
        inputs: ["linkerscript"]
        recursive: true
        scan: {
            console.debug("scanning linkerscript " + filePath + " for dependencies");
            var retval = [];
            var linkerScript = new TextFile(filePath, TextFile.ReadOnly);
            var regexp = /[\s]*INCLUDE[\s]+(\S+).*/ // "INCLUDE filename"
            var match;
            while (!linkerScript.atEof()) {
                match = regexp.exec(linkerScript.readLine());
                if (match) {
                    var dependencyFileName = match[1];
                    retval.push(dependencyFileName);
                    console.debug("linkerscript " + filePath + " depends on " + dependencyFileName);
                }
            }
            linkerScript.close();
            return retval;
        }
        searchPaths: {
            var retval = [];
            for (var i = 0; i < (product.cpp.libraryPaths || []).length; i++)
                retval.push(product.cpp.libraryPaths[i]);
            var regexp = /[\s]*SEARCH_DIR\((\S+)\).*/ // "SEARCH_DIR(path)"
            var match;
            var linkerScript = new TextFile(input.filePath, TextFile.ReadOnly);
            while (!linkerScript.atEof()) {
                match = regexp.exec(linkerScript.readLine());
                if(match) {
                    var additionalPath = match[1];
                    retval.push(additionalPath);
                }
            }
            linkerScript.close();
            return retval;
        }
    }
}
