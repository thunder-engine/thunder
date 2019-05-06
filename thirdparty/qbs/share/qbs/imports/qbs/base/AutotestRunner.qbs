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

import qbs.File
import qbs.FileInfo
import qbs.ModUtils

Product {
    name: "autotest-runner"
    type: ["autotest-result"]
    builtByDefault: false
    property stringList arguments: []
    property stringList environment: ModUtils.flattenDictionary(qbs.commonRunEnvironment)
    property bool limitToSubProject: true
    property stringList wrapper: []
    property string workingDir
    property stringList auxiliaryInputs

    Depends {
        productTypes: "autotest"
        limitToSubProject: product.limitToSubProject
    }
    Depends {
        productTypes: auxiliaryInputs
        limitToSubProject: product.limitToSubProject
    }

    Rule {
        inputsFromDependencies: "application"
        auxiliaryInputs: product.auxiliaryInputs
        outputFileTags: "autotest-result"
        prepare: {
            // TODO: This is hacky. Possible solution: Add autotest tag to application
            // in autotest module and have that as inputsFromDependencies instead of application.
            if (!input.product.type.contains("autotest")) {
                var cmd = new JavaScriptCommand();
                cmd.silent = true;
                return cmd;
            }
            var commandFilePath;
            var installed = input.moduleProperty("qbs", "install");
            if (installed)
                commandFilePath = ModUtils.artifactInstalledFilePath(input);
            if (!commandFilePath || !File.exists(commandFilePath))
                commandFilePath = input.filePath;
            var workingDir = product.workingDir ? product.workingDir
                                                : FileInfo.path(commandFilePath);
            var arguments = product.arguments;
            var allowFailure = false;
            if (input.autotest) {
                // FIXME: We'd like to let the user override with an empty list, but
                //        qbscore turns undefined lists into empty ones at the moment.
                if (input.autotest.arguments && input.autotest.arguments.length > 0)
                    arguments = input.autotest.arguments;

                if (input.autotest.workingDir)
                    workingDir = input.autotest.workingDir;
                allowFailure = input.autotest.allowFailure;
            }
            var fullCommandLine = product.wrapper
                .concat([commandFilePath])
                .concat(arguments);
            var cmd = new Command(fullCommandLine[0], fullCommandLine.slice(1));
            cmd.description = "Running test " + input.fileName;
            cmd.environment = product.environment;
            cmd.workingDirectory = workingDir;
            if (allowFailure)
                cmd.maxExitCode = 32767;
            return cmd;
        }
    }
}
