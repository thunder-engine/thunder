/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

import qbs.Environment
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Probes
import qbs.Utilities

Module {
    Probes.PathProbe {
        id: qnxSdkProbe
        names: ["qnx700", "qnx660", "qnx650"]
        searchPaths: qbs.hostOS.contains("windows")
                      ? [Environment.getEnv("USERPROFILE"), Environment.getEnv("SystemDrive")]
                      : [Environment.getEnv("HOME"), "/opt"]
    }

    Probe {
        id: qnxTargetOsProbe
        property string qnxSdkDir: sdkDir
        property stringList targets: []
        configure: {
            if (qnxSdkDir) {
                var validEntries = [];
                var entries = File.directoryEntries(
                            FileInfo.joinPaths(qnxSdkDir, "target"),
                            File.Dirs | File.NoDotAndDotDot);
                for (var i = 0; i < entries.length; ++i) {
                    if (/^qnx[0-9]$/.test(entries[i]))
                        validEntries.push(entries[i]);
                }
                validEntries.sort();
                validEntries.reverse();
                targets = validEntries;
                found = targets.length > 0;
            } else {
                found = false;
            }
        }
    }

    version: qnxSdkProbe.found ? qnxSdkProbe.fileName.substr(3, 3).split("").join(".") : undefined

    readonly property bool qnx7: version ? Utilities.versionCompare(version, "7") >= 0 : false

    property string sdkDir: qnxSdkProbe.filePath

    property string hostArch: qnx7 ? "x86_64" : "x86"

    property string hostOs: {
        if (qbs.hostOS.contains("linux"))
            return "linux";
        if (qbs.hostOS.contains("macos"))
            return "darwin";
        if (qbs.hostOS.contains("windows"))
            return qnx7 ? "win64" : "win32";
    }

    property string targetOs: qnxTargetOsProbe.targets[0]

    property string compilerName: "gcc"

    property string configurationDir: FileInfo.joinPaths(Environment.getEnv("HOME"), ".qnx")
    property string hostDir: FileInfo.joinPaths(sdkDir, "host", hostOs, hostArch)
    property string targetDir: FileInfo.joinPaths(sdkDir, "target", targetOs)

    property var buildEnv: ({
        "QNX_HOST": hostDir,
        "QNX_TARGET": targetDir,
        "QNX_CONFIGURATION": configurationDir
    })

    qbs.sysroot: targetDir

    validate: {
        if (!sdkDir) {
            throw ModUtils.ModuleError("Could not find a QNX SDK in any of the following "
                                       + "locations:\n\t" + qnxSdkProbe.candidatePaths.join("\n\t")
                                       + "\nInstall the QNX SDK to one of the above locations, "
                                       + "or set the qnx.sdkDir property to a valid QNX SDK "
                                       + "location.");
        }

        if (!hostOs) {
            throw ModUtils.ModuleError("Host operating system '" + qbs.hostOS
                                       + "' is not supported by the QNX SDK.");
        } else if (!File.exists(hostDir)) {
            throw ModUtils.ModuleError("Detected host tools operating system '" + hostOs
                                       + "' and architecture '" + hostArch + "' directory is not "
                                       + "present in the QNX SDK installed at '" + sdkDir
                                       + "' in the expected location '" + hostDir
                                       + "'; did you forget to install it?");
        }

        if (!targetOs)
            throw ModUtils.ModuleError("Could not find any QNX targets in '" + targetDir + "'");
    }
}
