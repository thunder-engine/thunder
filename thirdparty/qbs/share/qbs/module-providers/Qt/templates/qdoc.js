/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2015 Jake Petroules.
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

var FileInfo = require("qbs.FileInfo");
var ModUtils = require("qbs.ModUtils");

function qdocArgs(product, input, outputDir) {
    var args = [input.filePath];
    var qtVersion = ModUtils.moduleProperty(product, "versionMajor");
    if (qtVersion >= 5) {
        args.push("-outputdir");
        args.push(outputDir);
    }

    return args;
}

var _qdocDefaultFileTag = "qdoc-output";
function qdocFileTaggers() {
    var t = _qdocDefaultFileTag;
    return {
        ".qhp": [t, "qhp"],
        ".qhp.sha1": [t, "qhp-sha1"],
        ".css": [t, "qdoc-css"],
        ".html": [t, "qdoc-html"],
        ".index": [t, "qdoc-index"],
        ".png": [t, "qdoc-png"]
    };
}

function outputArtifacts(product, input) {
    var tracker = new ModUtils.BlackboxOutputArtifactTracker();
    tracker.hostOS = product.moduleProperty("qbs", "hostOS");
    tracker.shellPath = product.moduleProperty("qbs", "shellPath");
    tracker.defaultFileTags = [_qdocDefaultFileTag];
    tracker.fileTaggers = qdocFileTaggers();
    tracker.command = FileInfo.joinPaths(ModUtils.moduleProperty(product, "binPath"),
                                         ModUtils.moduleProperty(product, "qdocName"));
    tracker.commandArgsFunction = function (outputDirectory) {
        return qdocArgs(product, input, outputDirectory);
    };
    tracker.commandEnvironmentFunction = function (outputDirectory) {
        var env = {};
        var qdocEnv = ModUtils.moduleProperty(product, "qdocEnvironment");
        for (var j = 0; j < qdocEnv.length; ++j) {
            var e = qdocEnv[j];
            var idx = e.indexOf("=");
            var name = e.slice(0, idx);
            var value = e.slice(idx + 1, e.length);
            env[name] = value;
        }
        env["OUTDIR"] = outputDirectory; // Qt 4 replacement for -outputdir
        return env;
    };
    return tracker.artifacts(ModUtils.moduleProperty(product, "qdocOutputDir"));
}
