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

import qbs
import qbs.File
import qbs.FileInfo

Product {
    type: ["android.apk"]
    Depends { name: "Android.sdk" }

    property string packageName: name
    property bool automaticSources: true
    property bool legacyLayout: false

    property path sourceSetDir: legacyLayout ? undefined
                                             : FileInfo.joinPaths(sourceDirectory, "src/main")
    property path resourcesDir: FileInfo.joinPaths(sourceSetDir, "res")
    property path assetsDir: FileInfo.joinPaths(sourceSetDir, "assets")
    property path sourcesDir: FileInfo.joinPaths(sourceSetDir, legacyLayout ? "src" : "java")
    property path manifestFile: defaultManifestFile

    readonly property path defaultManifestFile: FileInfo.joinPaths(sourceSetDir,
                                                                   "AndroidManifest.xml")

    Group {
        name: "java sources"
        condition: product.automaticSources
        prefix: product.sourcesDir + '/'
        files: "**/*.java"
    }

    Group {
        name: "android resources"
        condition: product.automaticSources
        fileTags: ["android.resources"]
        prefix: product.resourcesDir + '/'
        files: "**/*"
    }

    Group {
        name: "android assets"
        condition: product.automaticSources
        fileTags: ["android.assets"]
        prefix: product.assetsDir + '/'
        files: "**/*"
    }

    Group {
        name: "manifest"
        condition: product.automaticSources
        fileTags: ["android.manifest"]
        files: manifestFile && manifestFile !== defaultManifestFile
               ? [manifestFile]
               : (File.exists(defaultManifestFile) ? [defaultManifestFile] : [])
    }
}
