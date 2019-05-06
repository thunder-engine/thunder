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

NativeBinary {
    type: isForAndroid && !consoleApplication ? ["android.apk"] : ["application"]

    property bool usesNativeCode

    Depends {
        // Note: If we are multiplexing, then this dependency is technically only needed in
        //       the aggregate. However, the user should not have to write the respective
        //       condition when assigning to properties of this module, so we load it
        //       regardless and turn off its rules for the native part of the build.
        name: "Android.sdk"
        condition: isForAndroid && !consoleApplication
    }
    Properties {
        condition: isForAndroid && !consoleApplication && !usesNativeCode
        multiplexByQbsProperties: []
        aggregate: false
    }
    Properties {
        condition: isForAndroid && !consoleApplication && usesNativeCode
                   && multiplexByQbsProperties && multiplexByQbsProperties.contains("architectures")
                   && qbs.architectures && qbs.architectures.length > 1
        aggregate: true
        multiplexedType: "android.nativelibrary"
    }
    aggregate: base
    multiplexByQbsProperties: base
    multiplexedType: base

    installDir: isBundle ? "Applications" : "bin"

    Group {
        condition: install
        fileTagsFilter: isBundle ? "bundle.content" : "application";
        qbs.install: true
        qbs.installDir: installDir
        qbs.installSourceBase: isBundle ? destinationDirectory : outer
    }
}
