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
import qbs.FileInfo
import qbs.Process
import qbs.TextFile
import qbs.Utilities
import '../QtModule.qbs' as QtModule
import 'quick.js' as QC

QtModule {
    qtModuleName: @name@
    Depends { name: "Qt"; submodules: @dependencies@.concat("qml-private") }

    hasLibrary: @has_library@
    architectures: @archs@
    targetPlatform: @targetPlatform@
    staticLibsDebug: @staticLibsDebug@
    staticLibsRelease: @staticLibsRelease@
    dynamicLibsDebug: @dynamicLibsDebug@
    dynamicLibsRelease: @dynamicLibsRelease@
    linkerFlagsDebug: @linkerFlagsDebug@
    linkerFlagsRelease: @linkerFlagsRelease@
    frameworksDebug: @frameworksDebug@
    frameworksRelease: @frameworksRelease@
    frameworkPathsDebug: @frameworkPathsDebug@
    frameworkPathsRelease: @frameworkPathsRelease@
    libNameForLinkerDebug: @libNameForLinkerDebug@
    libNameForLinkerRelease: @libNameForLinkerRelease@
    libFilePathDebug: @libFilePathDebug@
    libFilePathRelease: @libFilePathRelease@
    pluginTypes: @pluginTypes@
    moduleConfig: @moduleConfig@
    cpp.defines: @defines@
    cpp.includePaths: @includes@
    cpp.libraryPaths: @libraryPaths@
    @additionalContent@

    readonly property bool _compilerIsQmlCacheGen: Utilities.versionCompare(Qt.core.version,
                                                                            "5.11") >= 0
    readonly property string _generatedLoaderFileName: _compilerIsQmlCacheGen
                                                       ? "qmlcache_loader.cpp"
                                                       : "qtquickcompiler_loader.cpp"
    property string compilerBaseName: (_compilerIsQmlCacheGen ? "qmlcachegen" : "qtquickcompiler")
    property string compilerFilePath: FileInfo.joinPaths(Qt.core.binPath,
                                        compilerBaseName + product.cpp.executableSuffix)
    property bool compilerAvailable: File.exists(compilerFilePath);
    property bool useCompiler: compilerAvailable && !_compilerIsQmlCacheGen

    Scanner {
        condition: useCompiler
        inputs: 'qt.quick.qrc'
        searchPaths: [FileInfo.path(input.filePath)]
        scan: QC.scanQrc(input.filePath)
    }

    FileTagger {
        condition: useCompiler
        patterns: "*.qrc"
        fileTags: ["qt.quick.qrc"]
        priority: 100
    }

    Rule {
        condition: useCompiler
        inputs: ["qt.quick.qrc"]
        Artifact {
            filePath: input.fileName + ".json"
            fileTags: ["qt.quick.qrcinfo"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.silent = true;
            cmd.sourceCode = function() {
                var content = QC.contentFromQrc(input.filePath);
                content.qrcFilePath = input.filePath;

                var tf = new TextFile(output.filePath, TextFile.WriteOnly);
                tf.write(JSON.stringify(content));
                tf.close();
            }
            return cmd;
        }
    }

    Rule {
        condition: useCompiler
        inputs: ["qt.quick.qrcinfo"]
        outputFileTags: ["cpp", "qrc"]
        multiplex: true
        outputArtifacts: {
            var infos = [];
            inputs["qt.quick.qrcinfo"].forEach(function (input) {
                var tf = new TextFile(input.filePath, TextFile.ReadOnly);
                infos.push(JSON.parse(tf.readAll()));
                tf.close();
            });

            var result = [];
            infos.forEach(function (info) {
                if (info.newQrcFileName) {
                    result.push({
                        filePath: info.newQrcFileName,
                        fileTags: ["qrc"]
                    });
                }
                info.qmlJsFiles.forEach(function (qmlJsFile) {
                    result.push({
                        filePath: qmlJsFile.output,
                        fileTags: ["cpp"]
                    });
                });
            });
            result.push({
                filePath: product.Qt.quick._generatedLoaderFileName,
                fileTags: ["cpp"]
            });
            return result;
        }
        prepare: {
            var infos = [];
            inputs["qt.quick.qrcinfo"].forEach(function (input) {
                var tf = new TextFile(input.filePath, TextFile.ReadOnly);
                infos.push(JSON.parse(tf.readAll()));
                tf.close();
            });

            var cmds = [];
            var qmlCompiler = product.Qt.quick.compilerFilePath;
            var useCacheGen = product.Qt.quick._compilerIsQmlCacheGen;
            var cmd;
            var loaderFlags = [];

            function findOutput(fileName) {
                for (var k in outputs) {
                    for (var i in outputs[k]) {
                        if (outputs[k][i].fileName === fileName)
                            return outputs[k][i];
                    }
                }
                throw new Error("Qt Quick compiler rule: Cannot find output artifact "
                                + fileName + ".");
            }

            infos.forEach(function (info) {
                if (info.newQrcFileName) {
                    loaderFlags.push("--resource-file-mapping="
                                     + FileInfo.fileName(info.qrcFilePath)
                                     + ":" + info.newQrcFileName);
                    var args = ["--filter-resource-file",
                                info.qrcFilePath];
                    if (useCacheGen)
                        args.push("-o");
                    args.push(findOutput(info.newQrcFileName).filePath);
                    cmd = new Command(qmlCompiler, args);
                    cmd.description = "generating " + info.newQrcFileName;
                    cmds.push(cmd);
                } else {
                    loaderFlags.push("--resource-file-mapping=" + info.qrcFilePath);
                }
                info.qmlJsFiles.forEach(function (qmlJsFile) {
                    var args = ["--resource=" + info.qrcFilePath, qmlJsFile.input];
                    if (useCacheGen)
                        args.push("-o");
                    args.push(findOutput(qmlJsFile.output).filePath);
                    cmd = new Command(qmlCompiler, args);
                    cmd.description = "generating " + qmlJsFile.output;
                    cmd.workingDirectory = FileInfo.path(info.qrcFilePath);
                    cmds.push(cmd);
                });
            });

            var args = loaderFlags.concat(infos.map(function (info) { return info.qrcFilePath; }));
            if (useCacheGen)
                args.push("-o");
            args.push(findOutput(product.Qt.quick._generatedLoaderFileName).filePath);
            cmd = new Command(qmlCompiler, args);
            cmd.description = "generating loader source";
            cmds.push(cmd);
            return cmds;
        }
    }
}
