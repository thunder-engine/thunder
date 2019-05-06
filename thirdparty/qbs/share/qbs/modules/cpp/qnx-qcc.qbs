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

import qbs.FileInfo

UnixGCC {
    Depends { name: "qnx" }

    condition: qbs.targetOS.contains("qnx") &&
               qbs.toolchain && qbs.toolchain.contains("qcc")
    priority: 1

    distributionIncludePaths: FileInfo.joinPaths(qnx.targetDir, "usr", "include")

    toolchainInstallPath: FileInfo.joinPaths(qnx.hostDir, "usr", "bin")

    sysroot: qnx.targetDir
    sysrootFlags: sysroot ? [systemIncludeFlag + FileInfo.joinPaths(sysroot, "usr", "include")] : []

    cCompilerName: "qcc" + compilerExtension
    cxxCompilerName: (qnx.qnx7 ? "q++" : "QCC") + compilerExtension

    targetDriverFlags: qnxTarget ? ["-V" + qnxTarget] : []

    systemIncludeFlag: !qnx.qnx7 ? includeFlag : base

    property string qnxTarget: qbs.architecture
                               ? qnx.compilerName + "_" + targetSystem + qnxTargetArchName
                               : undefined

    property string qnxTargetArchName: {
        switch (qbs.architecture) {
        case "arm64":
            return "aarch64le";
        case "armv7a":
            return "armv7le";
        case "x86":
        case "x86_64":
            return qbs.architecture;
        }
    }

    // QNX doesn't support Objective-C or Objective-C++ and qcc/q++ don't use toolchainPrefix
    compilerPath: FileInfo.joinPaths(toolchainInstallPath, compilerName)
    compilerPathByLanguage: ({
        "c": FileInfo.joinPaths(toolchainInstallPath, cCompilerName),
        "cpp": FileInfo.joinPaths(toolchainInstallPath, cxxCompilerName),
        "objc": undefined,
        "objcpp": undefined,
        "asm_cpp": FileInfo.joinPaths(toolchainInstallPath, cCompilerName)
    })

    toolchainPrefix: target + "-"

    targetVendor: ["x86", "x86_64"].contains(qbs.architecture) ? "pc" : base
    targetSystem: "nto"
    targetAbi: "qnx" + qnx.version + (qnxTargetArchName === "armv7le" ? "eabi" : "")

    buildEnv: qnx.buildEnv
    probeEnv: buildEnv

    setupBuildEnvironment: {
        for (var key in product.cpp.buildEnv) {
            v = new ModUtils.EnvironmentVariable(key);
            v.value = product.cpp.buildEnv[key];
            v.set();
        }
    }
}
