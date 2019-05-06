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

import qbs.File
import "../../../modules/cpp/gcc.js" as Gcc

PathProbe {
    // Inputs
    property string compilerFilePath
    property var environment

    property string _nullDevice: qbs.nullDevice
    property stringList _toolchain: qbs.toolchain

    // Outputs
    property int versionMajor
    property int versionMinor
    property int versionPatch

    configure: {
        if (!File.exists(compilerFilePath)) {
            found = false;
            return;
        }

        var macros = Gcc.dumpMacros(environment, compilerFilePath, undefined, _nullDevice);

        if (_toolchain.contains("clang")) {
            versionMajor = parseInt(macros["__clang_major__"], 10);
            versionMinor = parseInt(macros["__clang_minor__"], 10);
            versionPatch = parseInt(macros["__clang_patchlevel__"], 10);
            found = true;
        } else {
            versionMajor = parseInt(macros["__GNUC__"], 10);
            versionMinor = parseInt(macros["__GNUC_MINOR__"], 10);
            versionPatch = parseInt(macros["__GNUC_PATCHLEVEL__"], 10);
            found = true;
        }
    }
}
