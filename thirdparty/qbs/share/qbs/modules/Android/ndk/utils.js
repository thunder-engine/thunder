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

function abiNameToDirName(abiName) {
    switch (abiName) {
    case "armeabi":
    case "armeabi-v7a":
        return "arm";
    case "arm64-v8a":
        return "arm64";
    default:
        return abiName;
    }
}

function androidAbi(arch) {
    switch (arch) {
    case "arm64":
        return "arm64-v8a";
    case "armv5":
    case "armv5te":
        return "armeabi";
    case "armv7":
    case "armv7a":
        return "armeabi-v7a";
    case "mips":
    case "mipsel":
        return "mips";
    case "mips64":
    case "mips64el":
        return "mips64";
    default:
        return arch;
    }
}

function commonCompilerFlags(toolchain, buildVariant, abi, armMode) {
    var flags = ["-ffunction-sections", "-funwind-tables",
                 "-Werror=format-security", "-fstack-protector-strong"];

    if (abi === "arm64-v8a")
        flags.push("-fpic");

    if (abi === "armeabi" || abi === "armeabi-v7a") {
        flags.push("-fpic");

        if (abi === "armeabi")
            flags.push("-mtune=xscale", "-msoft-float");

        if (abi === "armeabi-v7a") {
            flags.push("-mfpu=vfpv3-d16");
            flags.push("-mfloat-abi=softfp");
        }
    }

    if (abi === "mips" || abi === "mips64") {
        flags.push("-fpic", "-finline-functions", "-fmessage-length=0",
                   "-fno-inline-functions-called-once", "-fgcse-after-reload",
                   "-frerun-cse-after-loop", "-frename-registers");
    }

    if ((abi === "x86" || abi === "x86_64") && toolchain.contains("clang"))
        flags.push("-fPIC");

    if (armMode)
        flags.push("-m" + armMode);

    return flags;
}

function commonLinkerFlags(abi) {
    return ["-z", "noexecstack", "-z", "relro", "-z", "now"];
}

function getBinutilsPath(ndk, toolchainPrefix) {
    if (["x86", "x86_64"].contains(ndk.abi))
        return ndk.abi + "-" + ndk.toolchainVersionNumber;
    return toolchainPrefix + ndk.toolchainVersionNumber;
}
