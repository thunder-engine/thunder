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

import qbs.ModUtils
import qbs.TextFile
import qbs.Utilities
import qbs.WindowsUtils

import "setuprunenv.js" as SetupRunEnv

GenericGCC {
    condition: qbs.targetOS.contains("windows") &&
               qbs.toolchain && qbs.toolchain.contains("mingw")
    priority: 0
    staticLibraryPrefix: "lib"
    staticLibrarySuffix: ".a"
    dynamicLibrarySuffix: ".dll"
    executableSuffix: ".exe"
    debugInfoSuffix: ".debug"
    imageFormat: "pe"
    windowsApiCharacterSet: "unicode"
    platformDefines: base.concat(WindowsUtils.characterSetDefines(windowsApiCharacterSet))
                         .concat("WIN32")
    probeEnv: buildEnv

    Properties {
        condition: product.multiplexByQbsProperties.contains("buildVariants")
                   && qbs.buildVariants && qbs.buildVariants.length > 1
                   && qbs.buildVariant !== "release"
                   && product.type.containsAny(["staticlibrary", "dynamiclibrary"])
        variantSuffix: "d"
    }

    property string windresName: 'windres'
    property path windresPath: { return toolchainPathPrefix + windresName }

    setupBuildEnvironment: {
        var v = new ModUtils.EnvironmentVariable("PATH", product.qbs.pathListSeparator, true);
        v.prepend(product.cpp.toolchainInstallPath);
        v.set();
    }

    setupRunEnvironment: {
        var v = new ModUtils.EnvironmentVariable("PATH", product.qbs.pathListSeparator, true);
        v.prepend(product.cpp.toolchainInstallPath);
        v.set();
        SetupRunEnv.setupRunEnvironment(product, config);
    }

    FileTagger {
        patterns: ["*.manifest"]
        fileTags: ["native.pe.manifest"]
    }

    Rule {
        inputs: ["native.pe.manifest"]
        multiplex: true

        outputFileTags: ["rc"]
        outputArtifacts: {
            if (product.type.containsAny(["application", "dynamiclibrary"])) {
                return [{
                    filePath: input.completeBaseName + ".rc",
                    fileTags: ["rc"]
                }];
            }
            return [];
        }

        prepare: {
            var inputList = inputs["native.pe.manifest"];
            // TODO: Emulate manifest merging like Microsoft's mt.exe tool does
            if (inputList.length !== 1) {
                throw("The MinGW toolchain does not support manifest merging; " +
                      "you may only specify a single manifest file to embed into your assembly.");
            }

            var cmd = new JavaScriptCommand();
            cmd.silent = true;
            cmd.productType = product.type;
            cmd.inputFilePath = inputList[0].filePath;
            cmd.outputFilePath = output.filePath;
            cmd.sourceCode = function() {
                var tf;
                try {
                    tf = new TextFile(outputFilePath, TextFile.WriteOnly);
                    if (productType.contains("application"))
                        tf.write("1 "); // CREATEPROCESS_MANIFEST_RESOURCE_ID
                    else if (productType.contains("dynamiclibrary"))
                        tf.write("2 "); // ISOLATIONAWARE_MANIFEST_RESOURCE_ID
                    tf.write("24 "); // RT_MANIFEST
                    tf.writeLine(Utilities.cStringQuote(inputFilePath));
                } finally {
                    if (tf)
                        tf.close();
                }
            };
            return [cmd];
        }
    }

    FileTagger {
        patterns: ["*.rc"]
        fileTags: ["rc"]
    }

    Rule {
        inputs: ["rc"]
        auxiliaryInputs: ["hpp"]

        Artifact {
            filePath: Utilities.getHash(input.baseDir) + "/" + input.completeBaseName + "_res.o"
            fileTags: ["obj"]
        }

        prepare: {
            var platformDefines = input.cpp.platformDefines;
            var defines = input.cpp.defines;
            var includePaths = input.cpp.includePaths;
            var systemIncludePaths = input.cpp.systemIncludePaths;
            var args = [];
            var i;
            for (i in platformDefines) {
                args.push('-D');
                args.push(platformDefines[i]);
            }
            for (i in defines) {
                args.push('-D');
                args.push(defines[i]);
            }
            for (i in includePaths) {
                args.push('-I');
                args.push(includePaths[i]);
            }
            for (i in systemIncludePaths) {
                args.push('-I');
                args.push(systemIncludePaths[i]);
            }

            args = args.concat(['-i', input.filePath, '-o', output.filePath]);
            var cmd = new Command(product.cpp.windresPath, args);
            cmd.description = 'compiling ' + input.fileName;
            cmd.highlight = 'compiler';
            return cmd;
        }
    }
}

