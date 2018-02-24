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
import qbs.ModUtils
import qbs.Utilities

Product {
    name: "autotest-runner"
    type: ["autotest-result"]
    builtByDefault: false
    property stringList arguments: []
    property stringList environment: ModUtils.flattenDictionary(qbs.commonRunEnvironment)
    property bool limitToSubProject: true
    property stringList wrapper: []
    Depends {
        productTypes: "autotest"
        limitToSubProject: product.limitToSubProject
    }
    Rule {
        inputsFromDependencies: "application"
        Artifact {
            filePath: Utilities.getHash(input.filePath) + ".result.dummy" // Will never exist.
            fileTags: "autotest-result"
            alwaysUpdated: false
        }
        prepare: {
            var commandFilePath;
            var installed = input.moduleProperty("qbs", "install");
            if (installed)
                commandFilePath = ModUtils.artifactInstalledFilePath(input);
            if (!commandFilePath || !File.exists(commandFilePath))
                commandFilePath = input.filePath;
            var fullCommandLine = product.wrapper
                .concat([commandFilePath])
                .concat(product.arguments);
            var cmd = new Command(fullCommandLine[0], fullCommandLine.slice(1));
            cmd.description = "Running test " + input.fileName;
            cmd.environment = product.environment;
            return cmd;
        }
    }
}
