/****************************************************************************
**
** Copyright (C) 2019 Denis Shienkov <denis.shienkov@gmail.com>
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

var Cpp = require("cpp.js");
var Environment = require("qbs.Environment");
var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");
var ModUtils = require("qbs.ModUtils");
var Process = require("qbs.Process");
var TemporaryDir = require("qbs.TemporaryDir");
var TextFile = require("qbs.TextFile");
var Utilities = require("qbs.Utilities");
var WindowsUtils = require("qbs.WindowsUtils");

function guessArchitecture(macros)
{
    if (macros["__C51__"])
        return "mcs51";
    else if (macros["__CC_ARM"] === 1)
        return "arm";
}

function guessEndianness(macros)
{
    if (macros["__C51__"]) {
        // The 8051 processors are 8-bit. So, the data with an integer type
        // represented by more than one byte is stored as big endian in the
        // Keil toolchain. See for more info:
        // * http://www.keil.com/support/man/docs/c51/c51_ap_2bytescalar.htm
        // * http://www.keil.com/support/man/docs/c51/c51_ap_4bytescalar.htm
        return "big";
    } else if (macros["__ARMCC_VERSION"]) {
        return macros["__BIG_ENDIAN"] ? "big" : "little";
    }
}

function guessVersion(macros)
{
    if (macros["__C51__"]) {
        var version = macros["__C51__"];
        return { major: parseInt(version / 100),
            minor: parseInt(version % 100),
            patch: 0,
            found: true }
    } else if (macros["__CC_ARM"]) {
        var version = macros["__ARMCC_VERSION"];
        return { major: parseInt(version / 1000000),
            minor: parseInt(version / 10000) % 100,
            patch: parseInt(version) % 10000,
            found: true }
    }
}

// Note: The KEIL 8051 compiler does not support the predefined
// macros dumping. So, we do it with following trick where we try
// to compile a temporary file and to parse the console output.
function dumpC51CompilerMacros(compilerFilePath, qbs) {
    function createDumpMacrosFile() {
        var td = new TemporaryDir();
        var fn = FileInfo.fromNativeSeparators(td.path() + "/dump-macros.c");
        var tf = new TextFile(fn, TextFile.WriteOnly);
        tf.writeLine("#define VALUE_TO_STRING(x) #x");
        tf.writeLine("#define VALUE(x) VALUE_TO_STRING(x)");
        tf.writeLine("#define VAR_NAME_VALUE(var) \"\"\"|\"#var\"|\"VALUE(var)");
        tf.writeLine("#ifdef __C51__");
        tf.writeLine("#pragma message(VAR_NAME_VALUE(__C51__))");
        tf.writeLine("#endif");
        tf.close();
        return fn;
    }

    var fn = createDumpMacrosFile();
    var p = new Process();
    p.exec(compilerFilePath, [ fn ], false);
    var map = {};
    p.readStdOut().trim().split(/\r?\n/g).map(function(line) {
        var parts = line.split("\"|\"", 3);
        map[parts[1]] = parts[2];
    });
    return map;
}

function dumpArmCompilerMacros(compilerFilePath, qbs, nullDevice) {
    var p = new Process();
    p.exec(compilerFilePath,
           [ "-E", "--list-macros", nullDevice ],
           false);
    var map = {};
    p.readStdOut().trim().split(/\r?\n/g).map(function (line) {
        if (!line.startsWith("#define"))
            return;
        var parts = line.split(" ", 3);
        map[parts[1]] = parts[2];
    });
    return map;
}

function dumpMacros(compilerFilePath, qbs, nullDevice) {
    var map1 = dumpC51CompilerMacros(compilerFilePath, qbs);
    var map2 = dumpArmCompilerMacros(compilerFilePath, qbs, nullDevice);
    var map = {};
    for (var attrname in map1)
        map[attrname] = map1[attrname];
    for (var attrname in map2)
        map[attrname] = map2[attrname];
    return map;
}

function adjustPathsToWindowsSeparators(sourcePaths) {
    var resulingPaths = [];
    sourcePaths.forEach(function(path) {
        resulingPaths.push(FileInfo.toWindowsSeparators(path));
    });
    return resulingPaths;
}

function getMaxExitCode(architecture) {
    if (architecture === "mcs51")
        return 1;
    else if (architecture === "arm")
        return 0;
}

function collectLibraryDependencies(product) {
    var seen = {};
    var result = [];

    function addFilePath(filePath) {
        result.push({ filePath: filePath });
    }

    function addArtifactFilePaths(dep, artifacts) {
        if (!artifacts)
            return;
        var artifactFilePaths = artifacts.map(function(a) { return a.filePath; });
        artifactFilePaths.forEach(addFilePath);
    }

    function addExternalStaticLibs(obj) {
        if (!obj.cpp)
            return;
        function ensureArray(a) {
            return Array.isArray(a) ? a : [];
        }
        function sanitizedModuleListProperty(obj, moduleName, propertyName) {
            return ensureArray(ModUtils.sanitizedModuleProperty(obj, moduleName, propertyName));
        }
        var externalLibs = [].concat(
                    sanitizedModuleListProperty(obj, "cpp", "staticLibraries"));
        var staticLibrarySuffix = obj.moduleProperty("cpp", "staticLibrarySuffix");
        externalLibs.forEach(function(staticLibraryName) {
            if (!staticLibraryName.endsWith(staticLibrarySuffix))
                staticLibraryName += staticLibrarySuffix;
            addFilePath(staticLibraryName);
        });
    }

    function traverse(dep) {
        if (seen.hasOwnProperty(dep.name))
            return;
        seen[dep.name] = true;

        if (dep.parameters.cpp && dep.parameters.cpp.link === false)
            return;

        var staticLibraryArtifacts = dep.artifacts["staticlibrary"];
        if (staticLibraryArtifacts) {
            dep.dependencies.forEach(traverse);
            addArtifactFilePaths(dep, staticLibraryArtifacts);
            addExternalStaticLibs(dep);
        }
    }

    product.dependencies.forEach(traverse);
    addExternalStaticLibs(product);
    return result;
}

function filterStdOutput(cmd) {
    cmd.stdoutFilterFunction = function(output) {
        var sourceLines = output.split("\n");
        var filteredLines = [];
        for (var i in sourceLines) {
            if (sourceLines[i].startsWith("***")
                || sourceLines[i].startsWith(">>")
                || sourceLines[i].startsWith("    ")
                || sourceLines[i].startsWith("Program Size:")
                || sourceLines[i].startsWith("A51 FATAL")
                || sourceLines[i].startsWith("C51 FATAL")
                || sourceLines[i].startsWith("ASSEMBLER INVOKED BY")
                || sourceLines[i].startsWith("LOC  OBJ            LINE     SOURCE")
                ) {
                    filteredLines.push(sourceLines[i]);
            }
        }
        return filteredLines.join("\n");
    };
}

function compilerFlags(project, product, input, output, explicitlyDependsOn) {
    // Determine which C-language we"re compiling.
    var tag = ModUtils.fileTagForTargetLanguage(input.fileTags.concat(output.fileTags));
    var architecture = input.qbs.architecture;
    var args = [];

    var allDefines = [];
    var platformDefines = input.cpp.platformDefines;
    if (platformDefines)
        allDefines = allDefines.uniqueConcat(platformDefines);
    var defines = input.cpp.defines;
    if (defines)
        allDefines = allDefines.uniqueConcat(defines);

    var allIncludePaths = [];
    var includePaths = input.cpp.includePaths;
    if (includePaths)
        allIncludePaths = allIncludePaths.uniqueConcat(includePaths);
    var systemIncludePaths = input.cpp.systemIncludePaths;
    if (systemIncludePaths)
        allIncludePaths = allIncludePaths.uniqueConcat(systemIncludePaths);
    var compilerIncludePaths = input.cpp.compilerIncludePaths;
    if (compilerIncludePaths)
        allIncludePaths = allIncludePaths.uniqueConcat(compilerIncludePaths);

    if (architecture === "mcs51") {
        args.push(FileInfo.toWindowsSeparators(input.filePath));
        args.push("OBJECT (" + FileInfo.toWindowsSeparators(output.filePath) + ")");

        switch (input.cpp.optimization) {
        case "small":
            args.push("OPTIMIZE (SIZE)");
            break;
        case "fast":
            args.push("OPTIMIZE (SPEED)");
            break;
        case "none":
            args.push("OPTIMIZE (0)");
            break;
        }

        if (input.cpp.debugInformation)
            args.push("DEBUG");

        var warnings = input.cpp.warningLevel;
        if (warnings === "none") {
            args.push("WARNINGLEVEL (0)");
        } else if (warnings === "all") {
            args.push("WARNINGLEVEL (2)");
            args.push("FARWARNING");
        }

        if (allDefines.length > 0)
            args = args.concat("DEFINE (" + allDefines.join(",") + ")");

        if (allIncludePaths.length > 0) {
            var adjusted = adjustPathsToWindowsSeparators(allIncludePaths);
            args = args.concat("INCDIR (" + adjusted.join(";") + ")");
        }
    } else if (architecture === "arm") {
        args.push("-c", input.filePath);
        args.push("-o", output.filePath);

        switch (input.cpp.optimization) {
        case "small":
            args.push("-Ospace")
            break;
        case "fast":
            args.push("-Otime")
            break;
        case "none":
            args.push("-O0")
            break;
        }

        if (input.cpp.debugInformation) {
            args.push("--debug");
            args.push("-g");
        }

        var warnings = input.cpp.warningLevel;
        if (warnings === "none") {
            args.push("-W");
        } else if (warnings === "all") {
            // By default all warnings are enabled.
        }

        if (tag === "c") {
            // Note: Here we use the '==' operator because the '==='
            // operator does not work!
            if (input.cpp.cLanguageVersion == "c99")
                args.push("--c99");
        } else if (tag === "cpp") {
            args.push("--cpp");
            // Note: Here we use the '==' operator because the '==='
            // operator does not work!
            if (input.cpp.cxxLanguageVersion == "c++11")
                args.push("--cpp11");

            var enableExceptions = input.cpp.enableExceptions;
            if (enableExceptions !== undefined)
                args.push(enableExceptions ? "--exceptions" : "--no_exceptions");

            var enableRtti = input.cpp.enableRtti;
            if (enableRtti !== undefined)
                args.push(enableRtti ? "--rtti" : "--no_rtti");
        }

        args = args.concat(allDefines.map(function(define) { return '-D' + define }));
        args = args.concat(allIncludePaths.map(function(include) { return '-I' + include }));
    }

    args = args.concat(ModUtils.moduleProperty(input, "platformFlags"),
                       ModUtils.moduleProperty(input, "flags"),
                       ModUtils.moduleProperty(input, "platformFlags", tag),
                       ModUtils.moduleProperty(input, "flags", tag),
                       ModUtils.moduleProperty(input, "driverFlags", tag));
    return args;
}

function assemblerFlags(project, product, input, output, explicitlyDependsOn) {
    // Determine which C-language we"re compiling
    var tag = ModUtils.fileTagForTargetLanguage(input.fileTags.concat(output.fileTags));
    var architecture = input.qbs.architecture;
    var args = [];

    var allDefines = [];
    var platformDefines = input.cpp.platformDefines;
    if (platformDefines)
        allDefines = allDefines.uniqueConcat(platformDefines);
    var defines = input.cpp.defines;
    if (defines)
        allDefines = allDefines.uniqueConcat(defines);

    var allIncludePaths = [];
    var includePaths = input.cpp.includePaths;
    if (includePaths)
        allIncludePaths = allIncludePaths.uniqueConcat(includePaths);
    var systemIncludePaths = input.cpp.systemIncludePaths;
    if (systemIncludePaths)
        allIncludePaths = allIncludePaths.uniqueConcat(systemIncludePaths);
    var compilerIncludePaths = input.cpp.compilerIncludePaths;
    if (compilerIncludePaths)
        allIncludePaths = allIncludePaths.uniqueConcat(compilerIncludePaths);

    if (architecture === "mcs51") {
        args.push(FileInfo.toWindowsSeparators(input.filePath));
        args.push("OBJECT (" + FileInfo.toWindowsSeparators(output.filePath) + ")");

        if (input.cpp.debugInformation)
            args.push("DEBUG");

        if (allDefines.length > 0)
            args = args.concat("DEFINE (" + allDefines.join(",") + ")");

        if (allIncludePaths.length > 0) {
            var adjusted = adjustPathsToWindowsSeparators(allIncludePaths);
            args = args.concat("INCDIR (" + adjusted.join(";") + ")");
        }
    } else if (architecture === "arm") {
        args.push(input.filePath);
        args.push("-o", output.filePath);

        if (input.cpp.debugInformation) {
            args.push("--debug");
            args.push("-g");
        }

        var warnings = input.cpp.warningLevel;
        if (warnings === "none")
            args.push("--no_warn");

        var endianness = input.cpp.endianness;
        if (endianness)
            args.push((endianness === "little") ? "--littleend" : "--bigend");

        allDefines.forEach(function(define) {
            var parts = define.split("=");
            args.push("--pd");
            if (parts[1] === undefined)
                args.push(parts[0] + " SETA " + 1);
            else if (parts[1].contains("\""))
                args.push(parts[0] + " SETS " + parts[1]);
            else
                args.push(parts[0] + " SETA " + parts[1]);
        });

        args = args.concat(allIncludePaths.map(function(include) { return '-I' + include }));
    }

    args = args.concat(ModUtils.moduleProperty(input, "platformFlags", tag),
                       ModUtils.moduleProperty(input, "flags", tag),
                       ModUtils.moduleProperty(input, "driverFlags", tag));
    return args;
}

function linkerFlags(project, product, input, outputs) {
    var args = [];
    var architecture = product.qbs.architecture;

    if (architecture === "mcs51") {
        var allObjectPaths = [];
        function addObjectPath(obj) {
            allObjectPaths.push(obj.filePath);
        }

        if (inputs.obj)
            inputs.obj.map(function(obj) { addObjectPath(obj) });

        var libraryDependencies = collectLibraryDependencies(product);
        libraryDependencies.forEach(function(dep) { addObjectPath(dep); })

        var adjusted = adjustPathsToWindowsSeparators(allObjectPaths);
        args = args.concat(adjusted.join(","));

        // We need to wrap an output file name with quotes. Otherwise
        // the linker will ignore a specified file name.
        args.push("TO", '"' + FileInfo.toWindowsSeparators(outputs.application[0].filePath) + '"');

        if (!product.cpp.generateMapFile)
            args.push("NOMAP");
    } else if (architecture === "arm") {
        if (inputs.obj)
            args = args.concat(inputs.obj.map(function(obj) { return obj.filePath }));

        args.push("--output", outputs.application[0].filePath);

        if (product.cpp.generateMapFile)
            args.push("--list", outputs.map_file[0].filePath);

        var libraryPaths = product.cpp.libraryPaths;
        if (libraryPaths)
            args.push("--userlibpath=" + libraryPaths.join(","));

        var libraryDependencies = collectLibraryDependencies(product);
        args = args.concat(libraryDependencies.map(function(dep) { return dep.filePath; }));

        var linkerScripts = inputs.linkerscript
                ? inputs.linkerscript.map(function(a) { return a.filePath; }) : [];
        for (i in linkerScripts)
            args.push("--scatter", linkerScripts[i]);

        if (product.cpp.entryPoint)
            args.push("--entry", product.cpp.entryPoint);

        var debugInformation = product.cpp.debugInformation;
        if (debugInformation !== undefined)
            args.push(debugInformation ? "--debug" : "--no_debug");
    }

    args = args.concat(ModUtils.moduleProperty(product, "driverLinkerFlags"));
    return args;
}

function archiverFlags(project, product, input, outputs) {
    var args = [];
    var architecture = product.qbs.architecture;

    if (architecture === "mcs51") {
        args.push("TRANSFER");
        var allObjectPaths = [];

        function addObjectPath(obj) {
            allObjectPaths.push(obj.filePath);
        }

        if (inputs.obj)
            inputs.obj.map(function(obj) { addObjectPath(obj) });

        var adjusted = adjustPathsToWindowsSeparators(allObjectPaths);
        args = args.concat(adjusted.join(","));

        // We need to wrap a output file name with quotes. Otherwise
        // the linker will ignore a specified file name.
        args.push("TO", '"' + FileInfo.toWindowsSeparators(outputs.staticlibrary[0].filePath) + '"');
    } else if (architecture === "arm") {
        args.push("--create", outputs.staticlibrary[0].filePath);

        if (inputs.obj)
            args = args.concat(inputs.obj.map(function(obj) { return obj.filePath }));

        if (product.cpp.debugInformation)
            args.push("--debug_symbols");
    }

    return args;
}

function prepareCompiler(project, product, inputs, outputs, input, output, explicitlyDependsOn) {
    var args = compilerFlags(project, product, input, output, explicitlyDependsOn);
    var compilerPath = input.cpp.compilerPath;
    var architecture = input.cpp.architecture;
    var cmd = new Command(compilerPath, args)
    cmd.description = "compiling " + input.fileName;
    cmd.highlight = "compiler";
    cmd.maxExitCode = getMaxExitCode(architecture);
    filterStdOutput(cmd);
    return [cmd];
}


function prepareAssembler(project, product, inputs, outputs, input, output, explicitlyDependsOn) {
    var args = assemblerFlags(project, product, input, output, explicitlyDependsOn);
    var assemblerPath = input.cpp.assemblerPath;
    var cmd = new Command(assemblerPath, args)
    cmd.description = "assembling " + input.fileName;
    cmd.highlight = "compiler";
    filterStdOutput(cmd);
    return [cmd];
}

function prepareLinker(project, product, inputs, outputs, input, output) {
    var primaryOutput = outputs.application[0];
    var args = linkerFlags(project, product, input, outputs);
    var linkerPath = product.cpp.linkerPath;
    var cmd = new Command(linkerPath, args)
    cmd.description = "linking " + primaryOutput.fileName;
    cmd.highlight = "linker";
    filterStdOutput(cmd);
    return [cmd];
}

function prepareArchiver(project, product, inputs, outputs, input, output) {
    var args = archiverFlags(project, product, input, outputs);
    var archiverPath = product.cpp.archiverPath;
    var cmd = new Command(archiverPath, args)
    cmd.description = "linking " + output.fileName;
    cmd.highlight = "linker";
    filterStdOutput(cmd);
    return [cmd];
}
