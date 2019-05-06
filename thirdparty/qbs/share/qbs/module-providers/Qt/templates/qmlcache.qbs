import qbs.File
import qbs.FileInfo
import qbs.Process
import qbs.Utilities

Module {
    additionalProductTypes: ["qt.qml.qmlc", "qt.qml.jsc"]
    validate: {
        if (!qmlcachegenProbe.found)
            throw "qmlcachegen unsupported for this target";
    }
    property string qmlCacheGenPath: FileInfo.joinPaths(Qt.core.binPath, "qmlcachegen")
                                     + (qbs.hostOS.contains("windows") ? ".exe" : "")
    property bool supportsAllArchitectures: Utilities.versionCompare(Qt.core.version, "5.11") >= 0
    property string installDir

    readonly property stringList _targetArgs: {
        if (supportsAllArchitectures)
            return [];
        function translateArch(arch) {
            if (arch === "x86")
                return "i386";
            if (arch.startsWith("armv"))
                return "arm";
            return arch;
        }

        var args = ["--target-architecture", translateArch(qbs.architecture)];
        return args;
    }

    Depends { name: "Qt.core" }
    Probe {
        id: qmlcachegenProbe

        property string arch: qbs.architecture
        property string _qmlCacheGenPath: qmlCacheGenPath
        property stringList targetArgs: _targetArgs
        property bool _supportsAllArchitectures: supportsAllArchitectures

        configure: {
            if (_supportsAllArchitectures) {
                found = File.exists(_qmlCacheGenPath);
                return;
            }
            var process = new Process();
            found = false;
            try {
                found = process.exec(_qmlCacheGenPath,
                                     targetArgs.concat("--check-if-supported")) == 0;
                if (!found) {
                    var msg = "QML cache generation was requested but is unsupported on "
                               + "architecture '" + arch + "'.";
                    console.warn(msg);
                }
            } finally {
                process.close();
            }
        }
    }
    Rule {
        condition: qmlcachegenProbe.found
        inputs: ["qt.qml.qml", "qt.qml.js"]
        outputArtifacts: [{
            filePath: input.fileName + 'c',
            fileTags: input.fileTags.filter(function(tag) {
                    return tag === "qt.qml.qml" || tag === "qt.qml.js";
                })[0] + 'c'
        }]
        outputFileTags: ["qt.qml.qmlc", "qt.qml.jsc"]
        prepare: {
            var args = input.Qt.qmlcache._targetArgs.concat(input.filePath, "-o", output.filePath);
            var cmd = new Command(input.Qt.qmlcache.qmlCacheGenPath, args);
            cmd.description = "precompiling " + input.fileName;
            cmd.highlight = "compiler";
            return cmd;
        }
    }
    Group {
        condition: product.Qt.qmlcache.installDir !== undefined
        fileTagsFilter: product.Qt.qmlcache.additionalProductTypes
        qbs.install: true
        qbs.installDir: product.Qt.qmlcache.installDir
    }
}
