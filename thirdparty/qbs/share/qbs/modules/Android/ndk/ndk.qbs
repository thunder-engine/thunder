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
import qbs.Probes
import qbs.Utilities

import "utils.js" as NdkUtils

Module {
    Probes.AndroidNdkProbe {
        id: ndkProbe
        environmentPaths: (ndkDir ? [ndkDir] : []).concat(base)
    }

    version: ndkProbe.ndkVersion

    readonly property string abi: NdkUtils.androidAbi(qbs.architecture)
    PropertyOptions {
        name: "abi"
        description: "Corresponds to the 'APP_ABI' variable in an Android.mk file."
        allowedValues: ["arm64-v8a", "armeabi", "armeabi-v7a", "mips", "mips64", "x86", "x86_64"]
    }

    property string appStl: version && Utilities.versionCompare(version, "17") >= 0
                            ? "c++_shared" : "system"
    PropertyOptions {
        name: "appStl"
        description: "Corresponds to the 'APP_STL' variable in an Android.mk file."
        allowedValues: [
            "system", "gabi++_static", "gabi++_shared", "stlport_static", "stlport_shared",
            "gnustl_static", "gnustl_shared", "c++_static", "c++_shared"
        ]
    }

    property string toolchainVersion: latestToolchainVersion
    PropertyOptions {
        name: "toolchainVersion"
        description: "Corresponds to the 'NDK_TOOLCHAIN_VERSION' variable in an Android.mk file."
    }

    property string hostArch: ndkProbe.hostArch
    property string ndkDir: ndkProbe.path
    property string ndkSamplesDir: ndkProbe.samplesDir
    property string platform: ndkProbe.ndkPlatform

    property bool useUnifiedHeaders: version && Utilities.versionCompare(version, "15") >= 0

    // Internal properties.
    property stringList availableToolchains: ndkProbe.toolchains

    property stringList availableToolchainVersions: {
        var tcs = availableToolchains;
        var versions = [];
        for (var i = 0; i < tcs.length; ++i) {
            if ((qbs.toolchain.contains("clang") && tcs[i].startsWith("llvm-"))
                    || toolchainDirPrefixAbis.contains(tcs[i].split("-")[0])) {
                var re = /\-((?:[0-9]+)\.(?:[0-9]+))$/;
                var m = tcs[i].match(re);
                if (m)
                    versions.push(m[1]);
            }
        }

        // Sort by version number
        versions.sort(function (a, b) {
            var re = /^([0-9]+)\.([0-9]+)$/;
            a = a.match(re);
            a = {major: a[1], minor: a[2]};
            b = b.match(re);
            b = {major: b[1], minor: b[2]};
            if (a.major === b.major)
                return a.minor - b.minor;
            return a.major - b.major;
        });

        return versions;
    }

    property string latestToolchainVersion: availableToolchainVersions
                                            [availableToolchainVersions.length - 1]

    property int platformVersion: {
        if (platform) {
            var match = platform.match(/^android-([0-9]+)$/);
            if (match !== null) {
                return parseInt(match[1], 10);
            }
        }
    }

    property stringList abis: {
        var list = ["armeabi", "armeabi-v7a"];
        if (platformVersion >= 9)
            list.push("mips", "x86");
        if (platformVersion >= 21)
            list.push("arm64-v8a", "mips64", "x86_64");
        return list;
    }

    property stringList toolchainDirPrefixAbis: {
        var list = ["arm"];
        if (platformVersion >= 9)
            list.push("mipsel", "x86");
        if (platformVersion >= 21)
            list.push("aarch64", "mips64el", "x86_64");
        return list;
    }

    property string toolchainVersionNumber: {
        var prefix = "clang";
        if (toolchainVersion && toolchainVersion.startsWith(prefix))
            return toolchainVersion.substr(prefix.length);
        return toolchainVersion;
    }

    property string armMode: abi && abi.startsWith("armeabi")
            ? (qbs.buildVariant === "debug" ? "arm" : "thumb")
            : undefined;
    PropertyOptions {
        name: "armMode"
        description: "Determines the instruction set for armeabi configurations."
        allowedValues: ["arm", "thumb"]
    }

    property bool haveUnifiedStl: version && Utilities.versionCompare(version, "12") >= 0

    validate: {
        if (!ndkDir) {
            throw ModUtils.ModuleError("Could not find an Android NDK at any of the following "
                                       + "locations:\n\t" + ndkProbe.candidatePaths.join("\n\t")
                                       + "\nInstall the Android NDK to one of the above locations, "
                                       + "or set the Android.ndk.ndkDir property or "
                                       + "ANDROID_NDK_ROOT environment variable to a valid "
                                       + "Android NDK location.");
        }
        if (product.aggregate && !product.multiplexConfigurationId)
            return;
        var validator = new ModUtils.PropertyValidator("Android.ndk");
        validator.setRequiredProperty("abi", abi);
        validator.setRequiredProperty("appStl", appStl);
        validator.setRequiredProperty("toolchainVersion", toolchainVersion);
        validator.setRequiredProperty("hostArch", hostArch);
        validator.setRequiredProperty("platform", platform);
        validator.setRequiredProperty("toolchainVersionNumber", toolchainVersionNumber);
        return validator.validate();
    }
}
