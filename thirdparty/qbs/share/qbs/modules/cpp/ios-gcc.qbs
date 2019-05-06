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

import qbs.DarwinTools
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Utilities

DarwinGCC {
    priority: 1
    condition: qbs.targetOS.contains('ios') &&
               qbs.toolchain && qbs.toolchain.contains('gcc')

    targetSystem: "ios" + (minimumIosVersion || "")

    minimumDarwinVersion: minimumIosVersion
    minimumDarwinVersionCompilerFlag: qbs.targetOS.contains("ios-simulator")
                                      ? "-mios-simulator-version-min"
                                      : "-miphoneos-version-min"
    minimumDarwinVersionLinkerFlag: qbs.targetOS.contains("ios-simulator")
                                    ? "-ios_simulator_version_min"
                                    : "-iphoneos_version_min"

    libcxxAvailable: base
                     && minimumDarwinVersion
                     && Utilities.versionCompare(minimumDarwinVersion, "5") >= 0

    platformObjcFlags: base.concat(simulatorObjcFlags)
    platformObjcxxFlags: base.concat(simulatorObjcFlags)

    // Private properties
    readonly property stringList simulatorObjcFlags: {
        // default in Xcode and also required for building 32-bit Simulator binaries with ARC
        // since the default ABI version is 0 for 32-bit targets
        return qbs.targetOS.contains("ios-simulator")
                ? ["-fobjc-abi-version=2", "-fobjc-legacy-dispatch"]
                : [];
    }

    Rule {
        condition: !product.qbs.targetOS.contains("ios-simulator")
        inputsFromDependencies: ["bundle.content"]

        Artifact {
            filePath: FileInfo.joinPaths(product.destinationDirectory, product.targetName + ".ipa")
            fileTags: ["ipa"]
        }

        prepare: {
            var cmd = new Command("PackageApplication", [input.filePath, "-o", output.filePath]);
            cmd.description = "creating ipa";
            cmd.highlight = "codegen";
            return cmd;
        }
    }
}
