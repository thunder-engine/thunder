/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import qbs
import qbs.FileInfo
import qbs.Probes

Module {
    Depends { name: "cpp" }
    Depends { name: "pkgconfig"; required: false }

    property string theName: FileInfo.completeBaseName(filePath)

    Probes.PkgConfigProbe {
        id: pkgConfigProbe
        condition: pkgconfig.present
        name: theName
        executable: pkgconfig.executableFilePath
        libDirs: pkgconfig.libDirs
        forStaticBuild: pkgconfig.staticMode
    }

    Properties {
        condition: pkgConfigProbe.found
        version: pkgConfigProbe.modversion
        cpp.dynamicLibraries: pkgConfigProbe.libraries
        cpp.libraryPaths: pkgConfigProbe.libraryPaths
        cpp.includePaths: pkgConfigProbe.includePaths
        cpp.defines: pkgConfigProbe.defines
        cpp.driverLinkerFlags: pkgConfigProbe.linkerFlags
        cpp.commonCompilerFlags: pkgConfigProbe.compilerFlags
    }

    validate: {
        if (!pkgConfigProbe.found) {
            throw "Dependency '" + theName + "' not found for product '" + product.name + "'. "
                    + "Locating a package of this name via pkg-config also failed.";
        }
    }
}
