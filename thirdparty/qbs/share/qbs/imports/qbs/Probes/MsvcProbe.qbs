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

import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Utilities

PathProbe {
    // Inputs
    property string compilerFilePath
    property stringList enableDefinesByLanguage
    property string preferredArchitecture

    // Outputs
    property string architecture
    property int versionMajor
    property int versionMinor
    property int versionPatch
    property stringList includePaths
    property var buildEnv
    property var compilerDefinesByLanguage

    configure: {
        var languages = enableDefinesByLanguage;
        if (!languages || languages.length === 0)
            languages = ["c"];

        var info = languages.contains("c")
                ? Utilities.msvcCompilerInfo(compilerFilePath, "c") : {};
        var infoCpp = languages.contains("cpp")
                ? Utilities.msvcCompilerInfo(compilerFilePath, "cpp") : {};
        found = (!languages.contains("c") ||
                 (!!info && !!info.macros && !!info.buildEnvironment))
             && (!languages.contains("cpp") ||
                 (!!infoCpp && !!infoCpp.macros && !!infoCpp.buildEnvironment));

        compilerDefinesByLanguage = {
            "c": info.macros,
            "cpp": infoCpp.macros,
        };

        var macros = info.macros || infoCpp.macros;
        architecture = ModUtils.guessArchitecture(macros);

        var ver = macros["_MSC_FULL_VER"];

        versionMajor = parseInt(ver.substr(0, 2), 10);
        versionMinor = parseInt(ver.substr(2, 2), 10);
        versionPatch = parseInt(ver.substr(4), 10);

        buildEnv = info.buildEnvironment || infoCpp.buildEnvironment;
        var clParentDir = FileInfo.joinPaths(FileInfo.path(compilerFilePath), "..");
        var inclPath = FileInfo.joinPaths(clParentDir, "INCLUDE");
        if (!File.exists(inclPath))
            inclPath = FileInfo.joinPaths(clParentDir, "..", "INCLUDE");
        if (!File.exists(inclPath))
            inclPath = FileInfo.joinPaths(clParentDir, "..", "..", "INCLUDE");
        if (File.exists(inclPath))
            includePaths = [inclPath];

        if (preferredArchitecture && Utilities.canonicalArchitecture(preferredArchitecture)
                !== Utilities.canonicalArchitecture(architecture)) {
            throw "'" + preferredArchitecture +
                    "' differs from the architecture produced by this compiler (" +
                        architecture + ")";
        }
    }
}
