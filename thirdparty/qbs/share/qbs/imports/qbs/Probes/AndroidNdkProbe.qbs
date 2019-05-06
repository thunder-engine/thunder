/****************************************************************************
**
** Copyright (C) 2015 Jake Petroules.
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
import qbs.TextFile
import "../../../modules/Android/android-utils.js" as AndroidUtils

PathProbe {
    // Inputs
    property stringList hostOS: qbs.hostOS
    property path sdkPath

    environmentPaths: Environment.getEnv("ANDROID_NDK_ROOT")
    platformSearchPaths: {
        var paths = [];
        if (sdkPath)
            paths.push(FileInfo.joinPaths(sdkPath, "ndk-bundle"));
        if (qbs.hostOS.contains("windows"))
            paths.push(FileInfo.joinPaths(Environment.getEnv("LOCALAPPDATA"),
                                          "Android", "sdk", "ndk-bundle"));
        if (qbs.hostOS.contains("macos"))
            paths.push(FileInfo.joinPaths(Environment.getEnv("HOME"),
                                          "Library", "Android", "sdk", "ndk-bundle"));
        if (qbs.hostOS.contains("linux"))
            paths.push(FileInfo.joinPaths(Environment.getEnv("HOME"),
                                          "Android", "Sdk", "ndk-bundle"));
        return paths;
    }

    // Outputs
    property stringList candidatePaths
    property string samplesDir
    property var hostArch
    property stringList toolchains: []
    property string ndkVersion
    property string ndkPlatform

    configure: {
        function readFileContent(filePath) {
            var result = null;
            if (!File.exists(filePath))
                return result;
            try {
                var tf = new TextFile(filePath, TextFile.ReadOnly);
                result = tf.readAll();
            } finally {
                if (tf)
                    tf.close();
            }
            return result;
        }

        var i, j, allPaths = (environmentPaths || []).concat(platformSearchPaths || []);
        candidatePaths = allPaths;
        for (i in allPaths) {
            var platforms = [];
            if (hostOS.contains("windows"))
                platforms.push("windows-x86_64", "windows");
            if (hostOS.contains("darwin"))
                platforms.push("darwin-x86_64", "darwin-x86");
            if (hostOS.contains("linux"))
                platforms.push("linux-x86_64", "linux-x86");
            for (j in platforms) {
                if (File.exists(FileInfo.joinPaths(allPaths[i], "prebuilt", platforms[j]))) {
                    path = allPaths[i];
                    var ndkPlatforms = AndroidUtils.availablePlatforms(path);
                    if (ndkPlatforms.length > 0)
                        ndkPlatform = ndkPlatforms[ndkPlatforms.length - 1];
                    if (File.exists(FileInfo.joinPaths(path, "samples")))
                        samplesDir = FileInfo.joinPaths(path, "samples"); // removed in r11
                    hostArch = platforms[j];
                    toolchains = File.directoryEntries(FileInfo.joinPaths(path, "toolchains"),
                                                       File.Dirs | File.NoDotAndDotDot);

                    // NDK r11 and above
                    var content = readFileContent(path + "/source.properties");
                    if (content) {
                        var lines = content.trim().split(/\r?\n/g).filter(function (line) {
                            return line.length > 0;
                        });
                        for (var l = 0; l < lines.length; ++l) {
                            var m = lines[l].match(/^Pkg\.Revision\s*=\s*([0-9\.]+)$/);
                            if (m) {
                                ndkVersion = m[1];
                                found = true;
                                return;
                            }
                        }
                    }

                    // NDK r10 and below
                    var releaseTextFileCandidates = ["RELEASE.txt", "RELEASE.TXT"]
                            .map(function(v) { return FileInfo.joinPaths(path, v); })
                            .filter(File.exists);
                    content = releaseTextFileCandidates.length
                            ? readFileContent(releaseTextFileCandidates[0]) : null;
                    if (content) {
                        var m = content.trim().match(/^r([0-9]+[a-z]?).*/);
                        if (m) {
                            ndkVersion = m[1];
                            found = true;
                            return;
                        }
                    }
                }
            }
        }
    }
}
