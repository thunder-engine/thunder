import qbs
import qbs.File
import qbs.FileInfo
import qbs.Probes
import "protobuf.js" as HelperFunctions

Module {
    property string protocBinary: protocProbe.filePath
    property pathList importPaths: []

    property string outputDir: product.buildDirectory + "/protobuf"
    readonly property string protobufRoot: FileInfo.path(FileInfo.path(protocBinary))

    readonly property var baseValidate: {
        return function() {
            if (!File.exists(protocBinary))
                throw "Can't find protoc binary. Please set the protocBinary property or make sure it is found in PATH";
        }
    }

    FileTagger {
        patterns: ["*.proto"]
        fileTags: ["protobuf.input"];
    }

    Probes.BinaryProbe {
        id: protocProbe
        names: ["protoc"]
    }
}
