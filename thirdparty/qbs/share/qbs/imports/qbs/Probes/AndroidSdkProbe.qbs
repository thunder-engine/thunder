/****************************************************************************
**
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

import qbs.Environment
import qbs.File
import qbs.FileInfo
import "../../../modules/Android/sdk/utils.js" as SdkUtils
import "../../../modules/Android/android-utils.js" as AndroidUtils

BinaryProbe {
    environmentPaths: Environment.getEnv("ANDROID_HOME")
    platformSearchPaths: {
        if (qbs.hostOS.contains("windows"))
            return [FileInfo.joinPaths(Environment.getEnv("LOCALAPPDATA"), "Android", "sdk")];
        if (qbs.hostOS.contains("macos"))
            return [FileInfo.joinPaths(Environment.getEnv("HOME"), "Library", "Android", "sdk")];
        if (qbs.hostOS.contains("linux"))
            return [FileInfo.joinPaths(Environment.getEnv("HOME"), "Android", "Sdk")];
    }

    // Outputs
    property stringList candidatePaths
    property var buildToolsVersions
    property string buildToolsVersion
    property var platforms
    property string platform

    configure: {
        var suffixes = nameSuffixes || [""];
        var i, allPaths = (environmentPaths || []).concat(platformSearchPaths || []);
        candidatePaths = allPaths;
        for (i in allPaths) {
            for (var j in suffixes) {
                if (File.exists(FileInfo.joinPaths(allPaths[i],
                                                   "tools", "android" + suffixes[j]))) {
                    path = allPaths[i];
                    buildToolsVersions = SdkUtils.availableBuildToolsVersions(path)
                    buildToolsVersion = buildToolsVersions[buildToolsVersions.length - 1];
                    platforms = AndroidUtils.availablePlatforms(path)
                    platform = platforms[platforms.length - 1];
                    found = true;
                    return;
                }
            }
        }
    }
}
