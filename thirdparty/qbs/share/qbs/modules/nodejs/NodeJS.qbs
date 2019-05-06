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

import qbs.Environment
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Probes

Module {
    // JavaScript files which have been "processed" - currently this simply means "copied to output
    // directory" but might later include minification and obfuscation processing
    additionalProductTypes: ["nodejs_processed_js"].concat(applicationFile ? ["application"] : [])

    Probes.NodeJsProbe {
        id: nodejs
        searchPaths: toolchainInstallPath ? [toolchainInstallPath] : []
    }

    Probes.NpmProbe {
        id: npm
        searchPaths: toolchainInstallPath ? [toolchainInstallPath] : []
        interpreterPath: FileInfo.path(nodejs.filePath)
    }

    property path applicationFile
    PropertyOptions {
        name: "applicationFile"
        description: "file whose corresponding output will be executed when running the Node.js app"
    }

    Group {
        name: "Application file";
        prefix: product.sourceDirectory + '/'
        files: nodejs.applicationFile ? [nodejs.applicationFile] : []
    }

    property path toolchainInstallPath: {
        if (nodejs.path && npm.path && nodejs.path !== npm.path)
            throw("node and npm binaries do not belong to the same installation ("
                  + nodejs.path + " vs " + npm.path + ")");
        return nodejs.path || npm.path;
    }

    property path interpreterFileName: nodejs.fileName
    property path interpreterFilePath: nodejs.filePath

    property path packageManagerFileName: npm.fileName
    property path packageManagerFilePath: npm.filePath

    property path packageManagerBinPath: npm.npmBin
    property path packageManagerRootPath: npm.npmRoot
    property path packageManagerPrefixPath: npm.npmPrefix

    // private properties
    readonly property path compiledIntermediateDir: FileInfo.joinPaths(product.buildDirectory,
                                                                       "tmp", "nodejs.intermediate")

    setupBuildEnvironment: {
        var v = new ModUtils.EnvironmentVariable("PATH", product.qbs.pathListSeparator, product.qbs.hostOS.contains("windows"));
        v.prepend(product.nodejs.toolchainInstallPath);
        v.set();
    }

    setupRunEnvironment: {
        var v = new ModUtils.EnvironmentVariable("NODE_PATH", product.qbs.pathListSeparator, product.qbs.hostOS.contains("windows"));
        v.prepend(FileInfo.path(Environment.getEnv("QBS_RUN_FILE_PATH")));
        v.set();
    }

    FileTagger {
        patterns: ["*.js"]
        fileTags: ["js"]
    }

    validate: {
        var validator = new ModUtils.PropertyValidator("nodejs");
        validator.setRequiredProperty("toolchainInstallPath", toolchainInstallPath);
        validator.setRequiredProperty("interpreterFileName", interpreterFileName);
        validator.setRequiredProperty("interpreterFilePath", interpreterFilePath);
        validator.setRequiredProperty("packageManagerFileName", packageManagerFileName);
        validator.setRequiredProperty("packageManagerFilePath", packageManagerFilePath);
        validator.setRequiredProperty("packageManagerBinPath", packageManagerBinPath);
        validator.setRequiredProperty("packageManagerRootPath", packageManagerRootPath);
        validator.setRequiredProperty("packageManagerPrefixPath", packageManagerPrefixPath);
        validator.validate();
    }

    Rule {
        inputs: ["js"]

        outputArtifacts: {
            var tags = ["nodejs_processed_js"];
            if (input.fileTags.contains("application_js") ||
                product.moduleProperty("nodejs", "applicationFile") === input.filePath)
                tags.push("application");

            // Preserve directory structure of input files
            var intermediatePath = product.sourceDirectory;

            // Handle nodejs.compiledIntermediateDir (QBS-5 workaround)
            var compiled = product.moduleProperty("nodejs", "compiledIntermediateDir");
            if (input.filePath.startsWith(compiled)) {
                intermediatePath = compiled;
            }

            intermediatePath = FileInfo.path(FileInfo.relativePath(intermediatePath,
                                                                   input.filePath));

            return [{
                filePath: FileInfo.joinPaths(product.destinationDirectory, intermediatePath,
                                             input.fileName),
                fileTags: tags
            }];
        }

        outputFileTags: ["nodejs_processed_js", "application"]

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "copying " + input.fileName;
            cmd.sourceCode = function() {
                File.copy(input.filePath, output.filePath);
            };
            return cmd;
        }
    }
}
