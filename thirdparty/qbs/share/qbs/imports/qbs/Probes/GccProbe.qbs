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
import qbs.ModUtils
import "../../../modules/cpp/gcc.js" as Gcc

PathProbe {
    // Inputs
    property var compilerFilePathByLanguage
    property stringList enableDefinesByLanguage
    property stringList flags: []
    property var environment

    property string _nullDevice: qbs.nullDevice
    property string _pathListSeparator: qbs.pathListSeparator
    property string _sysroot: qbs.sysroot
    property stringList _targetOS: qbs.targetOS

    // Outputs
    property string architecture
    property string endianness
    property string targetPlatform
    property stringList includePaths
    property stringList libraryPaths
    property stringList frameworkPaths
    property var compilerDefinesByLanguage

    configure: {
        compilerDefinesByLanguage = {};
        var languages = enableDefinesByLanguage;
        if (!languages || languages.length === 0)
            languages = ["c"];
        for (var i = 0; i < languages.length; ++i) {
            var fp = compilerFilePathByLanguage[languages[i]];
            if (fp && File.exists(fp)) {
                try {
                    compilerDefinesByLanguage[languages[i]] = Gcc.dumpMacros(environment, fp,
                                                                             flags, _nullDevice,
                                                                             languages[i]);
                } catch (e) {
                    // Only throw errors when determining the compiler defines for the C language;
                    // for other languages we presume it is an indication that the language is not
                    // installed (as is typically the case for Objective-C/C++ on non-Apple systems)
                    if (languages[i] === "c")
                        throw e;
                }
            } else if (languages[i] === "c") {
                found = false;
                return;
            }
        }

        var macros = compilerDefinesByLanguage["c"]
                  || compilerDefinesByLanguage["cpp"]
                  || compilerDefinesByLanguage["objc"]
                  || compilerDefinesByLanguage["objcpp"];
        var defaultPaths = Gcc.dumpDefaultPaths(environment, compilerFilePathByLanguage["cpp"] ||
                                                compilerFilePathByLanguage["c"],
                                                flags, _nullDevice,
                                                _pathListSeparator, _sysroot, _targetOS);
        found = !!macros && !!defaultPaths;

        includePaths = defaultPaths.includePaths;
        libraryPaths = defaultPaths.libraryPaths;
        frameworkPaths = defaultPaths.frameworkPaths;

        // We have to dump the compiler's macros; -dumpmachine is not suitable because it is not
        // always complete (for example, the subarch is not included for arm architectures).
        architecture = ModUtils.guessArchitecture(macros);
        targetPlatform = ModUtils.guessTargetPlatform(macros);

        switch (macros["__BYTE_ORDER__"]) {
            case "__ORDER_BIG_ENDIAN__":
                endianness = "big";
                break;
            case "__ORDER_LITTLE_ENDIAN__":
                endianness = "little";
                break;
        }
    }
}
