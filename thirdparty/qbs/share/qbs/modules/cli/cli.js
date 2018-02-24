/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2015 Jake Petroules.
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

var FileInfo = require("qbs.FileInfo");

function prepareCompiler(product, inputs, output) {
    var i;
    var args = ["/nologo"];

    var platformDefines = product.moduleProperty("cli", "platformDefines");
    var compilerDefines = product.moduleProperty("cli", "compilerDefines");
    var defines = product.moduleProperty("cli", "defines");
    var platformCompilerFlags = product.moduleProperty("cli", "platformCompilerFlags");
    var compilerFlags = product.moduleProperty("cli", "compilerFlags")
    var libraryPaths = product.moduleProperty("cli", "libraryPaths");
    var dynamicLibraries = product.moduleProperty("cli", "dynamicLibraries");
    var netmodules = product.moduleProperty("cli", "netmodules");
    var warnAsError = product.moduleProperty("cli", "treatWarningsAsErrors");
    var warningLevel = product.moduleProperty("cli", "warningLevel");
    var debugInformation = product.moduleProperty("cli", "debugInformation");
    var optimization = product.moduleProperty("cli", "optimization");
    var architecture = product.moduleProperty("cli", "architecture");

    var csharpCompilerPath = product.moduleProperty("cli", "csharpCompilerPath");
    var vbCompilerPath = product.moduleProperty("cli", "vbCompilerPath");
    var fsharpCompilerPath = product.moduleProperty("cli", "fsharpCompilerPath");

    var compilers = {
        "cli.csharp": csharpCompilerPath,
        "cli.vb": vbCompilerPath,
        "cli.fsharp": fsharpCompilerPath
    };

    var pathFunction = product.moduleProperty("qbs", "hostOS").contains("windows")
            ? FileInfo.toWindowsSeparators
            : function (path) { return path; };

    var outputDescription = "assembly";
    if (output.fileTags.contains("application")) {
        args.push("/target:" + (product.consoleApplication === false ? "winexe" : "exe"));
    } else if (output.fileTags.contains("dynamiclibrary")) {
        args.push("/target:library");
    } else if (output.fileTags.contains("cli.netmodule")) {
        args.push("/target:module");
        outputDescription = "netmodule";
    }

    // Make sure our input files are all the same language
    var keys = Object.keys(inputs);
    var language;
    for (i in keys) {
        if (Object.keys(compilers).contains(keys[i])) {
            if (language)
                throw("You cannot compile source files in more than one CLI language into a single " + outputDescription + ".");
            language = keys[i];
        }
    }

    if (!compilers[language]) {
        throw("No CLI compiler available to compile " + language + " files.");
    }

    // docs state "/platform is not available in the development environment in Visual C# Express"
    // does this mean from the IDE or the compiler itself shipped with Express does not support it?
    if (architecture !== undefined) {
        if (architecture === "x86" ||
            architecture === "anycpu" ||
            architecture === "anycpu32bitpreferred") // requires .NET 4.5
            ;
        else if (architecture === "x86_64")
            architecture = "x64";
        else if (architecture === "ia64")
            architecture = "Itanium";
        else if (architecture === "arm")
            architecture = "ARM";
        else
            throw("Invalid CLI architecture: " + architecture);

        args.push("/platform:" + architecture);
    }

    if (debugInformation !== undefined)
        args.push("/debug" + (debugInformation ? "+" : "-"));

    if (optimization !== undefined)
        args.push("/optimize" + (optimization !== "none" ? "+" : "-"));

    if (warnAsError !== undefined)
        args.push("/warnaserror" + (warnAsError ? "+" : "-"));

    if (warningLevel !== undefined) {
        if (language === "cli.vb") {
            if (warningLevel === "none" || warningLevel === "0")
                args.push("/quiet");
        } else {
            if (warningLevel === "all")
                warningLevel = "4";
            else if (warningLevel === "none")
                warningLevel = "0";
            args.push("/warn:" + warningLevel);
        }
    }

    // Preprocessor defines
    var allDefines = (platformDefines || []).concat(compilerDefines || []).concat(defines || []);
    if (allDefines.length > 0)
        args.push("/define:" + allDefines.join(";"));

    // Library search paths
    for (i in libraryPaths)
        args.push("/lib:" + libraryPaths.join(","));

    // Dependent libraries
    for (i in dynamicLibraries) {
        args.push("/reference:"
                  + dynamicLibraries[i]
                  + product.moduleProperty("cli", "dynamicLibrarySuffix"));
    }

    for (i in inputs.dynamiclibrary)
        args.push("/reference:" + inputs.dynamiclibrary[i].filePath);

    // Dependent netmodules
    for (i in netmodules) {
        args.push("/addmodule:" + netmodules.map(function (f) {
            return f + product.moduleProperty("cli", "netmoduleSuffix");
        }).join(";"));
    }

    if (inputs["cli.netmodule"]) {
        args.push("/addmodule:" + inputs["cli.netmodule"].map(function (f) {
            return f.filePath;
        }).join(";"));
    }

    // Dependent resources
    for (i in inputs["cli.resources"])
        args.push("/resource:" + pathFunction(inputs["cli.resources"][i].filePath));

    // Additional compiler flags
    args = args.concat((platformCompilerFlags || []).concat(compilerFlags || []));

    args.push("/out:" + pathFunction(output.filePath));

    for (i in inputs[keys[0]])
        args.push(pathFunction(inputs[keys[0]][i].filePath));

    var cmd = new Command(compilers[language], args);
    cmd.description = "linking " + output.fileName;
    cmd.highlight = "linker";
    cmd.workingDirectory = FileInfo.path(output.filePath);
    return cmd;
}
