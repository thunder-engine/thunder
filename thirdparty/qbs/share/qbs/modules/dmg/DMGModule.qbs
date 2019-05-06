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

import qbs.DarwinTools
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Process
import qbs.TextFile
import "dmg.js" as Dmg

Module {
    Depends { name: "xcode"; required: false }

    condition: qbs.hostOS.contains("darwin") && qbs.targetOS.contains("darwin")

    property string volumeName: product.targetName
    PropertyOptions {
        name: "volumeName"
        description: "the name of the disk image (displayed in Finder when mounted)"
    }

    property bool badgeVolumeIcon: false
    PropertyOptions {
        name: "badgeVolumeIcon"
        description: "whether to render the user-supplied icon on top of the " +
                     "default volume icon instead of using it directly"
    }

    property string format: "UDBZ"
    PropertyOptions {
        name: "format"
        description: "the format to create the disk image in"
    }

    property int compressionLevel: qbs.buildVariant === "release" ? 9 : undefined
    PropertyOptions {
        name: "compressionLevel"
        description: "sets the zlib or bzip2 compression level for UDZO and UDBZ disk images"
    }

    property string textutilPath: "/usr/bin/textutil"
    property string hdiutilPath: "/usr/bin/hdiutil"
    property string dmgSuffix: ".dmg"

    property string sourceBase

    readonly property string pythonPath: File.canonicalFilePath(FileInfo.joinPaths(path,
                                                                "..", "..",
                                                                "python"))

    property string backgroundColor
    property int iconSize: 128
    property int windowX: 100
    property int windowY: 100
    property int windowWidth: 640
    property int windowHeight: 480
    property var iconPositions

    property int iconX: windowWidth / 2
    property int iconY: windowHeight / 2

    property string defaultLicenseLocale
    property string licenseLocale
    property string licenseLanguageName
    property string licenseAgreeButtonText
    property string licenseDisagreeButtonText
    property string licensePrintButtonText
    property string licenseSaveButtonText
    property string licenseInstructionText

    FileTagger {
        patterns: [
            "*.txt", "*.rtf", "*.html", "*.doc", "*.docx", "*.odt", "*.xml", "*.webarchive",
            "LICENSE"
        ]
        fileTags: ["dmg.license.input"]
    }

    FileTagger {
        patterns: ["*.icns"]
        fileTags: ["icns"]
    }

    FileTagger {
        patterns: ["*.tif", "*.tiff"]
        fileTags: ["tiff"]
    }

    Rule {
        inputs: ["dmg.license.input"]

        outputFileTags: ["dmg.license"]
        outputArtifacts: ([{
            filePath: FileInfo.joinPaths(product.destinationDirectory, "licenses",
                                         FileInfo.relativePath(product.sourceDirectory,
                                                               input.filePath) + ".rtf"),
            fileTags: ["dmg.license"],
            dmg: {
                licenseLocale: input.dmg.licenseLocale,
                licenseLanguageName: input.dmg.licenseLanguageName,
                licenseAgreeButtonText: input.dmg.licenseAgreeButtonText,
                licenseDisagreeButtonText: input.dmg.licenseDisagreeButtonText,
                licensePrintButtonText: input.dmg.licensePrintButtonText,
                licenseSaveButtonText: input.dmg.licenseSaveButtonText,
                licenseInstructionText: input.dmg.licenseInstructionText
            }
        }])

        prepare: Dmg.prepareLicense.apply(Dmg, arguments)
    }

    Rule {
        multiplex: true
        inputs: ["dmg.input", "dmg.license", "icns", "tiff"]

        Artifact {
            fileTags: ["dmg.dmg"]
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         product.targetName + product.dmg.dmgSuffix)
        }

        prepare: Dmg.prepareDmg.apply(Dmg, arguments)
    }
}
