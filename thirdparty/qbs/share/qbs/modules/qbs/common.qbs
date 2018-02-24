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

import qbs 1.0
import qbs.Environment
import qbs.FileInfo
import qbs.ModUtils
import qbs.PathTools
import qbs.Utilities

Module {
    readonly property string configurationName: "default"
    property string defaultBuildVariant: {
        switch (configurationName.toLowerCase()) {
        case "release":
            return "release";
        default:
            return "debug";
        }
    }

    property string buildVariant: defaultBuildVariant

    property bool enableDebugCode: buildVariant == "debug"
    property bool debugInformation: (buildVariant == "debug")
    property string optimization: (buildVariant == "debug" ? "none" : "fast")
    readonly property stringList hostOS: undefined // set internally
    property string hostOSVersion: {
        if (hostOS && hostOS.contains("macos")) {
            return Utilities.getNativeSetting("/System/Library/CoreServices/ServerVersion.plist", "ProductVersion") ||
                   Utilities.getNativeSetting("/System/Library/CoreServices/SystemVersion.plist", "ProductVersion");
        } else if (hostOS && hostOS.contains("windows")) {
            var version = Utilities.getNativeSetting(windowsRegistryKey, "CurrentVersion");
            return version + "." + hostOSBuildVersion;
        }
    }

    property string hostOSBuildVersion: {
        if (hostOS.contains("macos")) {
            return Utilities.getNativeSetting("/System/Library/CoreServices/ServerVersion.plist", "ProductBuildVersion") ||
                   Utilities.getNativeSetting("/System/Library/CoreServices/SystemVersion.plist", "ProductBuildVersion");
        } else if (hostOS.contains("windows")) {
            return Utilities.getNativeSetting(windowsRegistryKey, "CurrentBuildNumber");
        }
    }

    readonly property var hostOSVersionParts: hostOSVersion ? hostOSVersion.split('.').map(function(item) { return parseInt(item, 10); }) : []
    readonly property int hostOSVersionMajor: hostOSVersionParts[0] || 0
    readonly property int hostOSVersionMinor: hostOSVersionParts[1] || 0
    readonly property int hostOSVersionPatch: hostOSVersionParts[2] || 0

    property stringList targetOS: hostOS
    property string pathListSeparator: hostOS.contains("windows") ? ";" : ":"
    property string pathSeparator: hostOS.contains("windows") ? "\\" : "/"
    property string nullDevice: hostOS.contains("windows") ? "NUL" : "/dev/null"
    property path shellPath: hostOS.contains("windows") ? windowsShellPath : "/bin/sh"
    property string profile
    property stringList toolchain: {
        var tc;
        if (targetOS.contains("windows"))
            tc = hostOS.contains("windows") ? "msvc" : "mingw";
        else if (targetOS.contains("darwin"))
            tc = hostOS.contains("macos") ? "xcode" : "clang";
        else if (targetOS.contains("freebsd"))
            tc = "clang";
        else if (targetOS.contains("qnx"))
            tc = "qcc";
        else
            tc = "gcc";
        return Utilities.canonicalToolchain(tc);
    }
    property string architecture
    property bool install: false
    property path installSourceBase
    property string installRoot: project.buildDirectory + "/install-root"
    property string installDir
    property string installPrefix: ""
    property path sysroot

    PropertyOptions {
        name: "buildVariant"
        allowedValues: ['debug', 'release']
        description: "name of the build variant"
    }

    PropertyOptions {
        name: "optimization"
        allowedValues: ['none', 'fast', 'small']
        description: "optimization level"
    }

    validate: {
        var validator = new ModUtils.PropertyValidator("qbs");
        validator.setRequiredProperty("hostOS", hostOS);
        validator.setRequiredProperty("targetOS", targetOS);
        validator.addCustomValidator("targetOS", targetOS, function (value) {
            if (!value || (value.contains("osx") && !value.contains("macos")))
                return false;
            return true;
        }, "the value 'osx' has been replaced by 'macos'; use that instead and update "
            + "hostOS and targetOS condition checks in your project accordingly");
        if (hostOS && (hostOS.contains("windows") || hostOS.contains("macos"))) {
            validator.setRequiredProperty("hostOSVersion", hostOSVersion,
                                          "could not detect host operating system version; " +
                                          "verify that system files and registry keys have not " +
                                          "been modified.");
            if (hostOSVersion)
                validator.addVersionValidator("hostOSVersion", hostOSVersion, 2, 4);

            validator.setRequiredProperty("hostOSBuildVersion", hostOSBuildVersion,
                                          "could not detect host operating system build version; " +
                                          "verify that system files or registry have not been " +
                                          "tampered with.");
        }

        validator.addCustomValidator("architecture", architecture, function (value) {
            return !architecture || architecture === Utilities.canonicalArchitecture(architecture);
        }, "'" + architecture + "' is invalid. You must use the canonical name '" +
        Utilities.canonicalArchitecture(architecture) + "'");

        validator.addCustomValidator("toolchain", toolchain, function (value) {
            if (toolchain === undefined)
                return false; // cannot have null toolchain, empty is valid... for now
            var canonical = Utilities.canonicalToolchain.apply(Utilities, toolchain);
            for (var i = 0; i < Math.max(canonical.length, toolchain.length); ++i) {
                if (canonical[i] !== toolchain[i])
                    return false;
            }
            return true;
        }, "'" + toolchain + "' is invalid. You must use the canonical list '" +
        Utilities.canonicalToolchain.apply(Utilities, toolchain) + "'");

        validator.addCustomValidator("toolchain", toolchain, function (value) {
            // None of the pairs listed here may appear in the same toolchain list.
            // Note that this check is applied AFTER canonicalization, so for example
            // {"clang", "msvc"} need not be checked, since a toolchain containing clang is
            // guaranteed to also contain gcc.
            var pairs = [
                ["gcc", "msvc"],
                ["llvm", "mingw"]
            ];
            var canonical = Utilities.canonicalToolchain.apply(Utilities, value);
            for (var i = 0; i < pairs.length; ++i) {
                if (canonical.contains(pairs[i][0]) && canonical.contains(pairs[i][1]))
                    return false;
            }
            return true;
        }, "'" + toolchain + "' contains one or more mutually exclusive toolchain types.");

        validator.validate();
    }

    // private properties
    property string windowsRegistryKey: "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion"
    property path windowsSystemRoot: FileInfo.fromWindowsSeparators(Utilities.getNativeSetting(windowsRegistryKey, "SystemRoot"))
    property path windowsShellPath: FileInfo.fromWindowsSeparators(Environment.getEnv("COMSPEC")) || FileInfo.joinPaths(windowsSystemRoot, "System32", "cmd.exe")
    property string windowsPathVariable: hostOS.contains("windows") ? "PATH" : "WINEPATH"

    property var commonRunEnvironment: {
        var env = Environment.currentEnv();
        if (targetOS.contains("windows")) {
            var newEntry = FileInfo.toWindowsSeparators(FileInfo.joinPaths(installRoot,
                                                                           installPrefix));
            env[windowsPathVariable] = PathTools.prependOrSetPath(newEntry,
                                                                  env[windowsPathVariable],
                                                                  qbs.pathListSeparator);
        } else if (hostOS.contains("macos") && targetOS.contains("macos")) {
            env["DYLD_FRAMEWORK_PATH"] = PathTools.prependOrSetPath([
                FileInfo.joinPaths(installRoot, installPrefix, "Library", "Frameworks"),
                FileInfo.joinPaths(installRoot, installPrefix, "lib"),
                FileInfo.joinPaths(installRoot, installPrefix)
            ].join(pathListSeparator), env["DYLD_FRAMEWORK_PATH"], qbs.pathListSeparator);
            env["DYLD_LIBRARY_PATH"] = PathTools.prependOrSetPath([
                FileInfo.joinPaths(installRoot, installPrefix, "lib"),
                FileInfo.joinPaths(installRoot, installPrefix, "Library", "Frameworks"),
                FileInfo.joinPaths(installRoot, installPrefix)
            ].join(pathListSeparator), env["DYLD_LIBRARY_PATH"], qbs.pathListSeparator);
        } else if (hostOS.contains("unix") && targetOS.contains("unix")) {
            env["LD_LIBRARY_PATH"] = PathTools.prependOrSetPath(
                FileInfo.joinPaths(installRoot, installPrefix, "lib"), env["LD_LIBRARY_PATH"],
                        qbs.pathListSeparator);
        }

        return env;
    }

    // Properties that can be set for multiplexing products.
    property stringList profiles
    property stringList architectures: {
        if (targetOS.contains("android"))
            return ["armv7a"]; // default should be armv5te for NDK < r16, but we can't check here
        if (targetOS.contains("ios-simulator"))
            return ["x86", "x86_64"];
        if (targetOS.contains("ios"))
            return ["armv7a", "arm64"];
        if (targetOS.contains("macos"))
            return ["x86_64"];
        if (targetOS.contains("tvos-simulator"))
            return ["x86_64"];
        if (targetOS.contains("tvos"))
            return ["arm64"];
        if (targetOS.contains("watchos-simulator"))
            return ["x86"];
        if (targetOS.contains("watchos"))
            return ["armv7k"];
        return architecture ? [architecture] : undefined;
    }

    property stringList buildVariants: [buildVariant]

    // internal properties
    readonly property string version: [versionMajor, versionMinor, versionPatch].join(".")
    readonly property int versionMajor: undefined // set internally
    readonly property int versionMinor: undefined // set internally
    readonly property int versionPatch: undefined // set internally
    readonly property var multiplexMap: ({
        profiles: "profile",
        architectures: "architecture",
        buildVariants: "buildVariant"
    })
}
