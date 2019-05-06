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

import qbs 1.0
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.PathTools
import qbs.Probes
import qbs.Utilities
import "iar.js" as IAR

CppModule {
    condition: qbs.toolchain && qbs.toolchain.contains("iar")

    Probes.BinaryProbe {
        id: compilerPathProbe
        condition: !toolchainInstallPath && !_skipAllChecks
        names: ["iccarm"]
    }

    Probes.IarProbe {
        id: iarProbe
        condition: !_skipAllChecks
        compilerFilePath: compilerPath
    }

    qbs.architecture: iarProbe.found ? iarProbe.architecture : original

    compilerVersionMajor: iarProbe.versionMajor
    compilerVersionMinor: iarProbe.versionMinor
    compilerVersionPatch: iarProbe.versionPatch
    endianness: iarProbe.endianness

    compilerDefinesByLanguage: []

    property string toolchainInstallPath: compilerPathProbe.found
        ? compilerPathProbe.path : undefined

    property string compilerExtension: qbs.hostOS.contains("windows") ? ".exe" : ""

    property bool generateMapFile: true
    PropertyOptions {
        name: "generateMapFile"
        description: "produce a linker list file (enabled by default)"
    }

    /* Work-around for QtCreator which expects these properties to exist. */
    property string cCompilerName: compilerName
    property string cxxCompilerName: compilerName

    compilerName: {
        switch (qbs.architecture) {
        case "arm":
            return "iccarm" + compilerExtension;
        case "mcs51":
            return "icc8051" + compilerExtension;
        case "avr":
            return "iccavr" + compilerExtension;
        }
    }
    compilerPath: FileInfo.joinPaths(toolchainInstallPath, compilerName)

    assemblerName: {
        switch (qbs.architecture) {
        case "arm":
            return "iasmarm" + compilerExtension;
        case "mcs51":
            return "a8051" + compilerExtension;
        case "avr":
            return "aavr" + compilerExtension;
        }
    }
    assemblerPath: FileInfo.joinPaths(toolchainInstallPath, assemblerName)

    linkerName: {
        switch (qbs.architecture) {
        case "arm":
            return "ilinkarm" + compilerExtension;
        case "mcs51":
            return "xlink" + compilerExtension;
        case "avr":
            return "xlink" + compilerExtension;
        }
    }
    linkerPath: FileInfo.joinPaths(toolchainInstallPath, linkerName)

    property string archiverName: {
        switch (qbs.architecture) {
        case "arm":
            return "iarchive" + compilerExtension;
        case "mcs51":
            return "xlib" + compilerExtension;
        case "avr":
            return "xlib" + compilerExtension;
        }
    }
    property string archiverPath: FileInfo.joinPaths(toolchainInstallPath, archiverName)

    runtimeLibrary: "static"

    staticLibrarySuffix: {
        switch (qbs.architecture) {
        case "arm":
            return ".a";
        case "mcs51":
            return ".r51";
        case "avr":
            return ".r90";
        }
    }

    executableSuffix: {
        switch (qbs.architecture) {
        case "arm":
            return ".out";
        case "mcs51":
            return qbs.debugInformation ? ".d51" : ".a51";
        case "avr":
            return qbs.debugInformation ? ".d90" : ".a90";
        }
    }

    property string objectSuffix: {
        switch (qbs.architecture) {
        case "arm":
            return ".o";
        case "mcs51":
            return ".r51";
        case "avr":
            return ".r90";
        }
    }

    imageFormat: {
        switch (qbs.architecture) {
        case "arm":
            return "elf";
        case "mcs51":
            return "ubrof";
        case "avr":
            return "ubrof";
        }
    }

    enableExceptions: false
    enableRtti: false

    Rule {
        id: assembler
        inputs: ["asm"]

        Artifact {
            fileTags: ["obj"]
            filePath: Utilities.getHash(input.baseDir) + "/"
                      + input.fileName + input.cpp.objectSuffix
        }

        prepare: IAR.prepareAssembler.apply(IAR, arguments);
    }

    FileTagger {
        condition: qbs.architecture === "arm";
        patterns: "*.s"
        fileTags: ["asm"]
    }

    FileTagger {
        condition: qbs.architecture === "mcs51";
        patterns: ["*.s51", "*.asm"]
        fileTags: ["asm"]
    }

    Rule {
        id: compiler
        inputs: ["cpp", "c"]
        auxiliaryInputs: ["hpp"]

        Artifact {
            fileTags: ["obj"]
            filePath: Utilities.getHash(input.baseDir) + "/"
                      + input.fileName + input.cpp.objectSuffix
        }

        prepare: IAR.prepareCompiler.apply(IAR, arguments);
    }

    Rule {
        id: applicationLinker
        multiplex: true
        inputs: ["obj", "linkerscript"]
        inputsFromDependencies: ["staticlibrary"]

        outputFileTags: {
            var tags = ["application"];
            if (product.moduleProperty("cpp", "generateMapFile"))
                tags.push("map_file");
            return tags;
        }
        outputArtifacts: {
            var app = {
                fileTags: ["application"],
                filePath: FileInfo.joinPaths(
                              product.destinationDirectory,
                              PathTools.applicationFilePath(product))
            };
            var artifacts = [app];
            if (product.cpp.generateMapFile) {
                artifacts.push({
                    fileTags: ["map_file"],
                filePath: FileInfo.joinPaths(
                              product.destinationDirectory,
                              product.targetName + ".map")
                });
            }
            return artifacts;
        }

        prepare:IAR.prepareLinker.apply(IAR, arguments);
    }

    Rule {
        id: staticLibraryLinker
        multiplex: true
        inputs: ["obj"]
        inputsFromDependencies: ["staticlibrary"]

        Artifact {
            fileTags: ["staticlibrary"]
            filePath: FileInfo.joinPaths(
                            product.destinationDirectory,
                            PathTools.staticLibraryFilePath(product))
        }

        prepare: IAR.prepareArchiver.apply(IAR, arguments);
    }
}
