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
import qbs.Utilities

Module {
    condition: qbs.targetOS.contains("windows")

    property path toolchainInstallPath: Utilities.getNativeSetting(registryKey)

    version: (versionMajor !== undefined && versionMinor !== undefined) ? (versionMajor + "." + versionMinor) : undefined
    property var versionParts: [ versionMajor, versionMinor, versionPatch, versionBuild ]
    property int versionMajor: Utilities.getNativeSetting(registryKey, "VersionMajor")
    property int versionMinor: Utilities.getNativeSetting(registryKey, "VersionMinor")
    property int versionPatch: Utilities.getNativeSetting(registryKey, "VersionBuild")
    property int versionBuild: Utilities.getNativeSetting(registryKey, "VersionRevision")

    property string compilerName: "makensis"
    property string compilerPath: compilerName

    property string warningLevel: "normal"
    PropertyOptions {
        name: "warningLevel"
        allowedValues: ["none", "normal", "errors", "warnings", "info", "all"]
    }

    property bool disableConfig: false
    PropertyOptions {
        name: "disableConfig"
        description: "disable inclusion of nsisconf.nsh"
    }

    property bool enableQbsDefines: true
    PropertyOptions {
        name: "enableQbsDefines"
        description: "built-in variables that are defined when using the NSIS compiler"
    }

    property stringList defines
    PropertyOptions {
        name: "defines"
        description: "variables that are defined when using the NSIS compiler"
    }

    property stringList compilerFlags
    PropertyOptions {
        name: "compilerFlags"
        description: "additional flags for the NSIS compiler"
    }

    property string compressor: "default"
    PropertyOptions {
        name: "compressor"
        description: "the compression algorithm used to compress files/data in the installer"
        allowedValues: ["default", "zlib", "zlib-solid", "bzip2", "bzip2-solid", "lzma", "lzma-solid"]
    }

    property string executableSuffix: ".exe"

    // Private properties
    property string registryKey: {
        if (!qbs.hostOS.contains("windows"))
            return undefined;

        var keys = [ "HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\NSIS" ];
        for (var i in keys) {
            if (Utilities.getNativeSetting(keys[i]))
                return keys[i];
        }
    }

    validate: {
        var validator = new ModUtils.PropertyValidator("nsis");

        // Only *require* the toolchain install path on Windows
        // On other (Unix-like) operating systems it'll probably be in the PATH
        if (qbs.targetOS.contains("windows"))
            validator.setRequiredProperty("toolchainInstallPath", toolchainInstallPath);

        validator.setRequiredProperty("versionMajor", versionMajor);
        validator.setRequiredProperty("versionMinor", versionMinor);
        validator.setRequiredProperty("versionPatch", versionPatch);
        validator.setRequiredProperty("versionBuild", versionBuild);
        validator.addVersionValidator("version", version, 2, 4);
        validator.addRangeValidator("versionMajor", versionMajor, 1);
        validator.addRangeValidator("versionMinor", versionMinor, 0);
        validator.addRangeValidator("versionPatch", versionPatch, 0);
        validator.addRangeValidator("versionBuild", versionBuild, 0);
        validator.validate();
    }

    setupBuildEnvironment: {
        if (toolchainInstallPath) {
            var v = new ModUtils.EnvironmentVariable("PATH", ";", true);
            v.prepend(product.nsis.toolchainInstallPath);
            v.prepend(FileInfo.joinPaths(product.nsis.toolchainInstallPath, "bin"));
            v.set();
        }
    }

    // NSIS Script File
    FileTagger {
        patterns: ["*.nsi"]
        fileTags: ["nsi"]
    }

    // NSIS Header File
    FileTagger {
        patterns: ["*.nsh"]
        fileTags: ["nsh"]
    }

    Rule {
        id: nsisCompiler
        multiplex: true
        inputs: ["nsi"]
        auxiliaryInputs: ["installable"]

        Artifact {
            fileTags: ["nsissetup", "application"]
            filePath: product.destinationDirectory + "/" + product.targetName + ModUtils.moduleProperty(product, "executableSuffix")
        }

        prepare: {
            var i;
            var args = [];

            // Prefix character for makensis options
            var opt = product.moduleProperty("qbs", "hostOS").contains("windows") ? "/" : "-";

            if (ModUtils.moduleProperty(product, "disableConfig")) {
                args.push(opt + "NOCONFIG");
            }

            var warningLevel = ModUtils.moduleProperty(product, "warningLevel");
            var warningLevels = ["none", "errors", "warnings", "info", "all"];
            if (warningLevel !== "normal") {
                var level = warningLevels.indexOf(warningLevel);
                if (level < 0) {
                    throw("Unexpected warning level '" + warningLevel + "'.");
                } else {
                    args.push(opt + "V" + level);
                }
            }

            var enableQbsDefines = ModUtils.moduleProperty(product, "enableQbsDefines")
            if (enableQbsDefines) {
                var map = {
                    "project.": project,
                    "product.": product
                };

                for (var prefix in map) {
                    var obj = map[prefix];
                    for (var prop in obj) {
                        var val = obj[prop];
                        if (typeof val !== 'function' && typeof val !== 'object' && !prop.startsWith("_")) {
                            args.push(opt + "D" + prefix + prop + "=" + val);
                        }
                    }
                }

                // Users are likely to need this
                var arch = product.moduleProperty("qbs", "architecture");
                args.push(opt + "Dqbs.architecture=" + arch);

                // Helper define for alternating between 32-bit and 64-bit logic
                if (arch === "x86_64" || arch === "ia64") {
                    args.push(opt + "DWin64");
                }
            }

            // User-supplied defines
            var defines = ModUtils.moduleProperty(product, "defines");
            for (i in defines) {
                args.push(opt + "D" + defines[i]);
            }

            // User-supplied flags
            var flags = ModUtils.moduleProperty(product, "compilerFlags");
            for (i in flags) {
                args.push(flags[i]);
            }

            // Override the compression algorithm if needed
            var compressor = ModUtils.moduleProperty(product, "compressor");
            if (compressor !== "default") {
                var compressorFlag = opt + "XSetCompressor /FINAL ";
                if (compressor.endsWith("-solid")) {
                    compressorFlag += "/SOLID ";
                }
                args.push(compressorFlag + compressor.split('-')[0]);
            }

            var inputFileNames = [];
            for (i in inputs.nsi) {
                inputFileNames.push(inputs.nsi[i].fileName);
                args.push(FileInfo.toNativeSeparators(inputs.nsi[i].filePath,
                                                      product.moduleProperty("qbs", "hostOS")));
            }

            // Output file name - this goes last to override any OutFile command in the script
            args.push(opt + "XOutFile " + output.filePath);

            var cmd = new Command(ModUtils.moduleProperty(product, "compilerPath"), args);
            cmd.description = "compiling " + inputFileNames.join(", ");
            cmd.highlight = "compiler";
            cmd.workingDirectory = FileInfo.path(output.filePath);
            return cmd;
        }
    }
}
