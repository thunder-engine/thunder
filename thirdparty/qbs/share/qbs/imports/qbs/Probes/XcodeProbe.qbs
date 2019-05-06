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

import qbs.File
import qbs.FileInfo
import qbs.Process
import qbs.PropertyList
import "../../../modules/xcode/xcode.js" as Xcode

Probe {
    // Inputs
    property string sdksPath
    property string developerPath
    property string xcodebuildPath
    property stringList targetOS: qbs.targetOS
    property string platformType
    property string platformPath
    property string devicePlatformPath
    property string _xcodeInfoPlist: FileInfo.joinPaths(developerPath, "..", "Info.plist")

    // Outputs
    property var architectureSettings
    property var availableSdks
    property string xcodeVersion

    configure: {
        if (File.exists(_xcodeInfoPlist)) {
            // Optimized case (no forking): reads CFBundleShortVersionString from
            // Xcode.app/Contents/Info.plist
            var propertyList = new PropertyList();
            try {
                propertyList.readFromFile(_xcodeInfoPlist);

                var plist = propertyList.toObject();
                if (plist)
                    xcodeVersion = plist["CFBundleShortVersionString"];
            } finally {
                propertyList.clear();
            }
        } else {
            // Fallback case: execute xcodebuild -version if Xcode.app/Contents/Info.plist is
            // missing; this can happen if developerPath is /, for example
            var process;
            try {
                process = new Process();
                process.exec(xcodebuildPath, ["-version"], true);
                var lines = process.readStdOut().trim().split(/\r?\n/g).filter(function (line) {
                    return line.length > 0;
                });
                for (var l = 0; l < lines.length; ++l) {
                    var m = lines[l].match(/^Xcode ([0-9\.]+)$/);
                    if (m) {
                        xcodeVersion = m[1];
                        break;
                    }
                }
            } finally {
                process.close();
            }
        }

        architectureSettings = {};
        var archSpecsPath = Xcode.archsSpecsPath(xcodeVersion, targetOS, platformType,
                                                 platformPath, devicePlatformPath);
        var archSpecsReader = new Xcode.XcodeArchSpecsReader(archSpecsPath);
        archSpecsReader.getArchitectureSettings().map(function (setting) {
            var val = archSpecsReader.getArchitectureSettingValue(setting);
            if (val)
                architectureSettings[setting] = val;
        });

        availableSdks = Xcode.sdkInfoList(sdksPath);
        found = !!xcodeVersion;
    }
}
