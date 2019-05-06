/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

import qbs.FileInfo
import qbs.ModUtils
import qbs.Probes

Module {
    condition: qbs.targetOS.contains("windows")

    Probes.InnoSetupProbe {
        id: innoSetupProbe
    }

    property path toolchainInstallPath: innoSetupProbe.path
    version: innoSetupProbe.version
    property var versionParts: version ? version.split('.').map(function (item) { return parseInt(item, 10); }) : []
    property int versionMajor: versionParts[0]
    property int versionMinor: versionParts[1]
    property int versionPatch: versionParts[2]

    property string compilerName: "ISCC.exe"
    property string compilerPath: FileInfo.joinPaths(toolchainInstallPath, compilerName)

    property bool verboseOutput: false
    PropertyOptions {
        name: "verboseOutput"
        description: "display verbose output from the Inno Setup compiler"
    }

    property pathList includePaths
    PropertyOptions {
        name: "includePaths"
        description: "directories to add to the include search path"
    }

    property stringList defines
    PropertyOptions {
        name: "defines"
        description: "variables that are defined when using the Inno Setup compiler"
    }

    property stringList compilerFlags
    PropertyOptions {
        name: "compilerFlags"
        description: "additional flags for the Inno Setup compiler"
    }

    readonly property string executableSuffix: ".exe"

    validate: {
        var validator = new ModUtils.PropertyValidator("innosetup");
        validator.setRequiredProperty("toolchainInstallPath", toolchainInstallPath);
        validator.setRequiredProperty("version", version);
        validator.setRequiredProperty("versionMajor", versionMajor);
        validator.setRequiredProperty("versionMinor", versionMinor);
        validator.setRequiredProperty("versionPatch", versionPatch);
        validator.addVersionValidator("version", version, 3, 3);
        validator.addRangeValidator("versionMajor", versionMajor, 1);
        validator.addRangeValidator("versionMinor", versionMinor, 0);
        validator.addRangeValidator("versionPatch", versionPatch, 0);
        validator.validate();
    }

    // Inno Setup Script
    FileTagger {
        patterns: ["*.iss"]
        fileTags: ["innosetup.iss"]
    }

    Rule {
        id: innoSetupCompiler
        inputs: ["innosetup.iss"]
        auxiliaryInputs: ["installable"]

        Artifact {
            fileTags: ["innosetup.exe", "application"]
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         product.targetName
                                            + ModUtils.moduleProperty(product, "executableSuffix"))
        }

        prepare: {
            var i;
            var args = [
                "/O" + FileInfo.toNativeSeparators(FileInfo.path(output.filePath)),
                "/F" + FileInfo.toNativeSeparators(FileInfo.completeBaseName(output.fileName))
            ];

            if (!ModUtils.moduleProperty(product, "verboseOutput"))
                args.push("/Q");

            var includePaths = ModUtils.moduleProperty(product, "includePaths");
            for (i in includePaths)
                args.push("/I" + FileInfo.toNativeSeparators(includePaths[i]));

            // User-supplied defines
            var defines = ModUtils.moduleProperty(product, "defines");
            for (i in defines)
                args.push("/D" + defines[i]);

            // User-supplied flags
            var flags = ModUtils.moduleProperty(product, "compilerFlags");
            for (i in flags)
                args.push(flags[i]);

            args.push(FileInfo.toNativeSeparators(input.filePath));
            var cmd = new Command(ModUtils.moduleProperty(product, "compilerPath"), args);
            cmd.description = "compiling " + input.fileName;
            cmd.highlight = "compiler";
            cmd.workingDirectory = FileInfo.path(input.filePath);
            return cmd;
        }
    }
}
