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
import qbs.TextFile
import qbs.Utilities
import "../../modules/Android/ndk/utils.js" as NdkUtils
import 'gcc.js' as Gcc

LinuxGCC {
    Depends { name: "Android.ndk" }

    condition: qbs.targetOS.contains("android") &&
               qbs.toolchain && qbs.toolchain.contains("gcc")
    priority: 2
    rpaths: [rpathOrigin]

    property string toolchainDir: {
        if (qbs.toolchain && qbs.toolchain.contains("clang")) {
            return Utilities.versionCompare(Android.ndk.version, "10") <= 0
                    ? "llvm-" + Android.ndk.toolchainVersionNumber : "llvm";
        }
        return NdkUtils.getBinutilsPath(Android.ndk, toolchainTriple + "-")
    }

    property string cxxStlBaseDir: FileInfo.joinPaths(Android.ndk.ndkDir, "sources", "cxx-stl")
    property string gabiBaseDir: FileInfo.joinPaths(cxxStlBaseDir, "gabi++")
    property string stlPortBaseDir: FileInfo.joinPaths(cxxStlBaseDir, "stlport")
    property string gnuStlBaseDir: FileInfo.joinPaths(cxxStlBaseDir, "gnu-libstdc++",
                                                      Android.ndk.toolchainVersionNumber)
    property string llvmStlBaseDir: FileInfo.joinPaths(cxxStlBaseDir, "llvm-libc++")
    property string stlBaseDir: {
        if (Android.ndk.appStl.startsWith("gabi++_"))
            return gabiBaseDir;
        else if (Android.ndk.appStl.startsWith("stlport_"))
            return stlPortBaseDir;
        else if (Android.ndk.appStl.startsWith("gnustl_"))
            return gnuStlBaseDir;
        else if (Android.ndk.appStl.startsWith("c++_"))
            return llvmStlBaseDir;
        return undefined;
    }

    property string stlLibsDir: {
        if (stlBaseDir) {
            var infix = Android.ndk.abi;
            if (Android.ndk.armMode === "thumb" && !Android.ndk.haveUnifiedStl)
                infix = FileInfo.joinPaths(infix, "thumb");
            return FileInfo.joinPaths(stlBaseDir, "libs", infix);
        }
        return undefined;
    }

    property string sharedStlFilePath: (stlLibsDir && Android.ndk.appStl.endsWith("_shared"))
        ? FileInfo.joinPaths(stlLibsDir, dynamicLibraryPrefix + Android.ndk.appStl + dynamicLibrarySuffix)
        : undefined
    property string staticStlFilePath: (stlLibsDir && Android.ndk.appStl.endsWith("_static"))
        ? FileInfo.joinPaths(stlLibsDir, staticLibraryPrefix + Android.ndk.appStl + staticLibrarySuffix)
        : undefined

    Group {
        name: "Android STL"
        condition: product.cpp.sharedStlFilePath && product.cpp.shouldLink
        files: product.cpp.sharedStlFilePath ? [product.cpp.sharedStlFilePath] : []
        fileTags: "android.stl"
    }

    Group {
        name: "gdbserver"
        condition: qbs.buildVariant !== "release" && product.cpp.shouldLink
        files: FileInfo.joinPaths(Android.ndk.ndkDir, "prebuilt",
                                  "android-" + NdkUtils.abiNameToDirName(Android.ndk.abi),
                                  "gdbserver", "gdbserver")
        fileTags: "android.gdbserver"
    }

    toolchainInstallPath: FileInfo.joinPaths(Android.ndk.ndkDir, "toolchains",
                                             toolchainDir, "prebuilt",
                                             Android.ndk.hostArch, "bin")

    property string toolchainTriple: [targetAbi === "androideabi" ? "arm" : targetArch,
                                      targetSystem, targetAbi].join("-")

    toolchainPrefix: {
        if (qbs.toolchain && qbs.toolchain.contains("clang"))
            return undefined;
        return toolchainTriple + "-";
    }

    machineType: {
        if (Android.ndk.abi === "armeabi")
            return "armv5te";
        if (Android.ndk.abi === "armeabi-v7a")
            return "armv7-a";
    }

    qbs.optimization: targetAbi === "androideabi" ? "small" : base

    enableExceptions: Android.ndk.appStl !== "system"
    enableRtti: Android.ndk.appStl !== "system"

    commonCompilerFlags: NdkUtils.commonCompilerFlags(qbs.toolchain, qbs.buildVariant,
                                                      Android.ndk.abi, Android.ndk.armMode)

    linkerFlags: NdkUtils.commonLinkerFlags(Android.ndk.abi)

    platformDriverFlags: ["-no-canonical-prefixes"]

    libraryPaths: {
        var prefix = FileInfo.joinPaths(sysroot, "usr");
        var paths = [];
        if (Android.ndk.abi === "mips64" || Android.ndk.abi === "x86_64") // no lib64 for arm64-v8a
            paths.push(FileInfo.joinPaths(prefix, "lib64"));
        paths.push(FileInfo.joinPaths(prefix, "lib"));
        return paths;
    }

    dynamicLibraries: {
        var libs = ["c", "m"];
        if (sharedStlFilePath)
            libs.push(sharedStlFilePath);
        return libs;
    }
    staticLibraries: {
        var libs = ["gcc"];
        if (staticStlFilePath) {
            libs.push(staticStlFilePath);
            if (Android.ndk.appStl === "c++_static") {
                var libAbi = FileInfo.joinPaths(stlLibsDir, "libc++abi.a");
                if (File.exists(libAbi))
                    libs.push(libAbi);
            }
        }
        return libs;
    }
    systemIncludePaths: {
        var includes = [];
        if (Android.ndk.useUnifiedHeaders) {
            // Might not be needed with Clang in a future NDK release
            includes.push(FileInfo.joinPaths(sysroot, "usr", "include", toolchainTriple));
        }
        if (Android.ndk.appStl === "system") {
            includes.push(FileInfo.joinPaths(cxxStlBaseDir, "system", "include"));
        } else if (Android.ndk.appStl.startsWith("gabi++")) {
            includes.push(FileInfo.joinPaths(gabiBaseDir, "include"));
        } else if (Android.ndk.appStl.startsWith("stlport")) {
            includes.push(FileInfo.joinPaths(stlPortBaseDir, "stlport"));
        } else if (Android.ndk.appStl.startsWith("gnustl")) {
            includes.push(FileInfo.joinPaths(gnuStlBaseDir, "include"));
            includes.push(FileInfo.joinPaths(gnuStlBaseDir, "libs", Android.ndk.abi, "include"));
            includes.push(FileInfo.joinPaths(gnuStlBaseDir, "include", "backward"));
        } else if (Android.ndk.appStl.startsWith("c++_")) {
            if (Utilities.versionCompare(Android.ndk.version, "13") >= 0) {
                includes.push(FileInfo.joinPaths(llvmStlBaseDir, "include"));
                includes.push(FileInfo.joinPaths(llvmStlBaseDir + "abi", "include"));
            } else {
                includes.push(FileInfo.joinPaths(llvmStlBaseDir, "libcxx", "include"));
                includes.push(FileInfo.joinPaths(llvmStlBaseDir + "abi", "libcxxabi", "include"));
            }
        }
        return includes;
    }
    defines: {
        var list = ["ANDROID"];
        if (Android.ndk.useUnifiedHeaders) {
            // Might be superseded by an -mandroid-version or similar Clang compiler flag in future
            list.push("__ANDROID_API__=" + Android.ndk.platformVersion);
        }
        return list;
    }
    binutilsPath: FileInfo.joinPaths(Android.ndk.ndkDir, "toolchains",
                                     NdkUtils.getBinutilsPath(Android.ndk, toolchainTriple + "-"),
                                     "prebuilt", Android.ndk.hostArch, "bin");
    binutilsPathPrefix: Gcc.pathPrefix(binutilsPath, toolchainTriple + "-")
    driverFlags: qbs.toolchain.contains("clang")
                 ? ["-gcc-toolchain", FileInfo.path(binutilsPath)].concat(base || []) : base
    syslibroot: FileInfo.joinPaths(Android.ndk.ndkDir, "platforms",
                                   Android.ndk.platform, "arch-"
                                   + NdkUtils.abiNameToDirName(Android.ndk.abi))
    sysroot: !Android.ndk.useUnifiedHeaders
             ? syslibroot
             : FileInfo.joinPaths(Android.ndk.ndkDir, "sysroot")

    targetArch: {
        switch (qbs.architecture) {
        case "arm64":
            return "aarch64";
        case "armv5":
        case "armv5te":
            return "armv5te";
        case "armv7a":
        case "x86_64":
            return qbs.architecture;
        case "x86":
            return "i686";
        case "mips":
        case "mipsel":
            return "mipsel";
        case "mips64":
        case "mips64el":
            return "mips64el";
        }
    }

    targetVendor: "none"
    targetSystem: "linux"
    targetAbi: "android" + (["armeabi", "armeabi-v7a"].contains(Android.ndk.abi) ? "eabi" : "")

    endianness: "little"

    Rule {
        condition: shouldLink
        inputs: "dynamiclibrary"
        Artifact {
            filePath: FileInfo.joinPaths("stripped-libs", input.fileName)
            fileTags: "android.nativelibrary"
        }
        prepare: {
            var stripArgs = ["--strip-unneeded", "-o", output.filePath, input.filePath];
            var stripCmd = new Command(product.cpp.stripPath, stripArgs);
            stripCmd.description = "Stripping unneeded symbols from " + input.fileName;
            return stripCmd;
        }
    }

    _skipAllChecks: !shouldLink

    validate: {
        if (_skipAllChecks)
            return;
        var baseValidator = new ModUtils.PropertyValidator("qbs");
        baseValidator.addCustomValidator("architecture", targetArch, function (value) {
            return value !== undefined;
        }, "unknown Android architecture '" + qbs.architecture + "'.");

        var validator = new ModUtils.PropertyValidator("cpp");
        validator.setRequiredProperty("targetArch", targetArch);

        return baseValidator.validate() && validator.validate();
    }
}
