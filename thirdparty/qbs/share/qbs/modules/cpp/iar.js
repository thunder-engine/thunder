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
    if (macros["__ICCARM__"] === "1")
        return "arm";
    else if (macros["__ICC8051__"] === "1")
        return "mcs51";
    else if (macros["__ICCAVR__"] === "1")
        return "avr";
}

function guessEndianness(macros)
{
    if (macros["__LITTLE_ENDIAN__"] === "1")
        return "little";
    return "big"
}

function dumpMacros(compilerFilePath, qbs, nullDevice) {
    var tempDir = new TemporaryDir();
    var inFilePath = FileInfo.fromNativeSeparators(tempDir.path() + "/empty-source.c");
    var inFile = new TextFile(inFilePath, TextFile.WriteOnly);
    var outFilePath = FileInfo.fromNativeSeparators(tempDir.path() + "/iar-macros.predef");
    var p = new Process();

    p.exec(compilerFilePath,
           [ inFilePath, "--predef_macros", outFilePath ],
           true);
    var outFile = new TextFile(outFilePath, TextFile.ReadOnly);
    var map = {};
    outFile.readAll().trim().split(/\r?\n/g).map(function (line) {
            var parts = line.split(" ", 3);
            map[parts[1]] = parts[2];
        });
    return map;
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

function compilerFlags(project, product, input, output, explicitlyDependsOn) {
    // Determine which C-language we"re compiling.
    var tag = ModUtils.fileTagForTargetLanguage(input.fileTags.concat(output.fileTags));

    var args = [];
    args.push(input.filePath);

    switch (input.cpp.optimization) {
    case "small":
        args.push("-Ohs");
        break;
    case "fast":
        args.push("-Ohz");
        break;
    case "none":
        args.push("-On");
        break;
    }

    if (input.cpp.debugInformation)
        args.push("--debug");

    var warnings = input.cpp.warningLevel;
    if (warnings === "none") {
        args.push("--no_warnings");
    } else if (warnings === "all") {
        args.push("--deprecated_feature_warnings="
            +"+attribute_syntax,"
            +"+preprocessor_extensions,"
            +"+segment_pragmas");
        if (tag === "cpp")
            args.push("--warn_about_c_style_casts");
    }
    if (input.cpp.treatWarningsAsErrors)
        args.push("--warnings_are_errors");

    // Choose byte order.
    var endianness = input.cpp.endianness;
    if (endianness) {
        if (input.qbs.architecture === "arm")
            args.push("--endian=" + endianness);
    }

    if (tag === "c") {
        // Language version.
        if (input.cpp.cLanguageVersion === "c89")
            args.push("--c89");
    } else if (tag === "cpp") {
        if (input.qbs.architecture === "arm") {
            args.push("--c++");
            if (!input.cpp.enableExceptions)
                args.push("--no_exceptions");
            if (!input.cpp.enableRtti)
                args.push("--no_rtti");
        } else if (input.qbs.architecture === "mcs51") {
            args.push("--ec++");
        } else if (input.qbs.architecture === "avr") {
            args.push("--ec++");
        }
    }

    var allDefines = [];
    var platformDefines = input.cpp.platformDefines;
    if (platformDefines)
        allDefines = allDefines.uniqueConcat(platformDefines);
    var defines = input.cpp.defines;
    if (defines)
        allDefines = allDefines.uniqueConcat(defines);
    args = args.concat(allDefines.map(function(define) { return "-D" + define }));

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
    args = args.concat(allIncludePaths.map(function(include) { return "-I" + include }));

    args.push("-o", output.filePath);

    args.push("--silent"); // Silent operation.

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

    var args = [];
    args.push(input.filePath);

    if (input.cpp.debugInformation)
        args.push("-r");

    var warnings = input.cpp.warningLevel;
    if (warnings === "none")
        args.push("-w-");
    else
        args.push("-w+");

    var allIncludePaths = [];
    var systemIncludePaths = input.cpp.systemIncludePaths;
    if (systemIncludePaths)
        allIncludePaths = allIncludePaths.uniqueConcat(systemIncludePaths);
    var compilerIncludePaths = input.cpp.compilerIncludePaths;
    if (compilerIncludePaths)
        allIncludePaths = allIncludePaths.uniqueConcat(compilerIncludePaths);
    args = args.concat(allIncludePaths.map(function(include) { return "-I" + include }));

    args.push("-o", output.filePath);

    args.push("-S"); // Silent operation.

    args = args.concat(ModUtils.moduleProperty(input, "platformFlags", tag),
                       ModUtils.moduleProperty(input, "flags", tag),
                       ModUtils.moduleProperty(input, "driverFlags", tag));
    return args;
}

function linkerFlags(project, product, input, outputs) {
    var i;
    var args = [];

    if (inputs.obj)
        args = args.concat(inputs.obj.map(function(obj) { return obj.filePath }));

    args.push("-o", outputs.application[0].filePath);

    if (product.cpp.generateMapFile) {
        if (product.qbs.architecture === "arm")
            args.push("--map", outputs.map_file[0].filePath);
        else if (product.qbs.architecture === "mcs51")
            args.push("-l", outputs.map_file[0].filePath);
        else if (product.qbs.architecture === "avr")
            args.push("-l", outputs.map_file[0].filePath);
    }

    var allLibraryPaths = [];
    var libraryPaths = product.cpp.libraryPaths;
    if (libraryPaths)
        allLibraryPaths = allLibraryPaths.uniqueConcat(libraryPaths);
    var distributionLibraryPaths = product.cpp.distributionLibraryPaths;
    if (distributionLibraryPaths)
        allLibraryPaths = allLibraryPaths.uniqueConcat(distributionLibraryPaths);
    args = args.concat(allLibraryPaths.map(function(path) { return '-L' + path }));

    var libraryDependencies = collectLibraryDependencies(product);
    if (libraryDependencies)
        args = args.concat(libraryDependencies.map(function(dep) { return dep.filePath }));

    if (product.cpp.debugInformation) {
        if (product.qbs.architecture === "mcs51")
            args.push("-rt");
        else if (product.qbs.architecture === "avr")
            args.push("-rt");
    }

    var linkerScripts = inputs.linkerscript
            ? inputs.linkerscript.map(function(a) { return a.filePath; }) : [];
    for (i in linkerScripts) {
        if (product.qbs.architecture === "arm")
            args.push("--config", linkerScripts[i]);
        else if (product.qbs.architecture === "mcs51")
            args.push("-f", linkerScripts[i]);
        else if (product.qbs.architecture === "avr")
            args.push("-f", linkerScripts[i]);
    }

    if (product.cpp.entryPoint) {
        if (product.qbs.architecture === "arm")
            args.push("--entry", product.cpp.entryPoint);
        else if (product.qbs.architecture === "mcs51")
            args.push("-s", product.cpp.entryPoint);
        else if (product.qbs.architecture === "avr")
            args.push("-s", product.cpp.entryPoint);
    }

    if (product.qbs.architecture === "arm")
        args.push("--silent"); // Silent operation.
    else if (product.qbs.architecture === "mcs51")
        args.push("-S"); // Silent operation.
    else if (product.qbs.architecture === "avr")
        args.push("-S"); // Silent operation.

    args = args.concat(ModUtils.moduleProperty(product, "driverLinkerFlags"));
    return args;
}

function archiverFlags(project, product, input, outputs) {
    var args = [];

    if (inputs.obj)
        args = args.concat(inputs.obj.map(function(obj) { return obj.filePath }));

    args.push("--create");
    args.push("-o", outputs.staticlibrary[0].filePath);

    return args;
}

function prepareCompiler(project, product, inputs, outputs, input, output, explicitlyDependsOn) {
    var args = compilerFlags(project, product, input, output, explicitlyDependsOn);
    var compilerPath = input.cpp.compilerPath;
    var cmd = new Command(compilerPath, args)
    cmd.description = "compiling " + input.fileName;
    cmd.highlight = "compiler";
    return [cmd];
}

function prepareAssembler(project, product, inputs, outputs, input, output, explicitlyDependsOn) {
    var args = assemblerFlags(project, product, input, output, explicitlyDependsOn);
    var assemblerPath = input.cpp.assemblerPath;
    var cmd = new Command(assemblerPath, args)
    cmd.description = "assembling " + input.fileName;
    cmd.highlight = "compiler";
    return [cmd];
}

function prepareLinker(project, product, inputs, outputs, input, output) {
    var primaryOutput = outputs.application[0];
    var args = linkerFlags(project, product, input, outputs);
    var linkerPath = product.cpp.linkerPath;
    var cmd = new Command(linkerPath, args)
    cmd.description = "linking " + primaryOutput.fileName;
    cmd.highlight = "linker";
    return [cmd];
}

function prepareArchiver(project, product, inputs, outputs, input, output) {
    var args = archiverFlags(project, product, input, outputs);
    var archiverPath = product.cpp.archiverPath;
    var cmd = new Command(archiverPath, args)
    cmd.description = "linking " + output.fileName;
    cmd.highlight = "linker";
    cmd.stdoutFilterFunction = function(output) {
        return "";
    };
    return [cmd];
}
