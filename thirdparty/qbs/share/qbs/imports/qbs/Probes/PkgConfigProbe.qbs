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

import qbs.Process
import qbs.FileInfo

Probe {
    // Inputs
    property string sysroot: qbs.sysroot
    property string executable: 'pkg-config'
    property string name
    property stringList packageNames: [name]
    property string minVersion
    property string exactVersion
    property string maxVersion
    property bool forStaticBuild: false
    property stringList libDirs // Full, non-sysrooted paths, mirroring the environment variable
    property string pathListSeparator: qbs.pathListSeparator

    // Output
    property stringList cflags // Unmodified --cflags output
    property stringList libs   // Unmodified --libs output

    property stringList defines
    property stringList libraries
    property stringList libraryPaths
    property stringList includePaths
    property stringList compilerFlags
    property stringList linkerFlags
    property string modversion

    configure: {
        if (!packageNames || packageNames.length === 0)
            throw 'PkgConfigProbe.packageNames must be specified.';
        var p = new Process();
        try {
            var libDirsToSet = libDirs;
            if (sysroot) {
                p.setEnv("PKG_CONFIG_SYSROOT_DIR", sysroot);
                if (!libDirsToSet) {
                    libDirsToSet = [
                        sysroot + "/usr/lib/pkgconfig",
                        sysroot + "/usr/share/pkgconfig"
                    ];
                }
            }
            if (libDirsToSet)
                p.setEnv("PKG_CONFIG_LIBDIR", libDirsToSet.join(pathListSeparator));
            var versionArgs = [];
            if (minVersion !== undefined)
                versionArgs.push("--atleast-version=" + minVersion);
            if (exactVersion !== undefined)
                versionArgs.push("--exact-version=" + exactVersion);
            if (maxVersion !== undefined)
                versionArgs.push("--max-version=" + maxVersion);
            if (versionArgs.length !== 0
                    && p.exec(executable, versionArgs.concat(packageNames)) !== 0) {
                return;
            }
            var args = packageNames;
            if (p.exec(executable, args.concat([ '--cflags' ])) === 0) {
                cflags = p.readStdOut().trim();
                cflags = cflags ? cflags.split(/\s/) : [];
                var libsArgs = args.concat("--libs");
                if (forStaticBuild)
                    libsArgs.push("--static");
                if (p.exec(executable, libsArgs) === 0) {
                    libs = p.readStdOut().trim();
                    libs = libs ? libs.split(/\s/) : [];
                    if (p.exec(executable, [packageNames[0]].concat([ '--modversion' ])) === 0) {
                        modversion = p.readStdOut().trim();
                        found = true;
                        includePaths = [];
                        defines = []
                        compilerFlags = [];
                        for (var i = 0; i < cflags.length; ++i) {
                            var flag = cflags[i];
                            if (flag.startsWith("-I"))
                                includePaths.push(flag.slice(2));
                            else if (flag.startsWith("-D"))
                                defines.push(flag.slice(2));
                            else
                                compilerFlags.push(flag);
                        }
                        libraries = [];
                        libraryPaths = [];
                        linkerFlags = [];
                        for (i = 0; i < libs.length; ++i) {
                            flag = libs[i];
                            if (flag.startsWith("-l"))
                                libraries.push(flag.slice(2));
                            else if (flag.startsWith("-L"))
                                libraryPaths.push(flag.slice(2));
                            else
                                linkerFlags.push(flag);
                        }
                        console.debug("PkgConfigProbe: found packages " + packageNames);
                        return;
                    }
                }
            }
            found = false;
            cflags = undefined;
            libs = undefined;
        } finally {
            p.close();
        }
    }
}
