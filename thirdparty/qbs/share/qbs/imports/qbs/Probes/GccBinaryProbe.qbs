import qbs.Environment
import qbs.FileInfo
import "path-probe.js" as PathProbeConfigure

BinaryProbe {
    // Inputs
    property string _compilerName
    property string _toolchainPrefix

    // Outputs
    property string tcPrefix

    platformSearchPaths: {
        var paths = base;
        if (qbs.targetOS.contains("windows") && qbs.hostOS.contains("windows"))
            paths.push(FileInfo.joinPaths(
                           Environment.getEnv("SystemDrive"), "MinGW", "bin"));
        return paths;
    }

    names: {
        var prefixes = [];
        if (_toolchainPrefix) {
            prefixes.push(_toolchainPrefix);
        } else {
            var arch = qbs.architecture;
            if (qbs.targetOS.contains("windows")) {
                if (!arch || arch === "x86") {
                    prefixes.push("mingw32-",
                                  "i686-w64-mingw32-",
                                  "i686-w64-mingw32.shared-",
                                  "i686-w64-mingw32.static-",
                                  "i686-mingw32-",
                                  "i586-mingw32msvc-");
                }
                if (!arch || arch === "x86_64") {
                    prefixes.push("x86_64-w64-mingw32-",
                                  "x86_64-w64-mingw32.shared-",
                                  "x86_64-w64-mingw32.static-",
                                  "amd64-mingw32msvc-");
                }
            }
        }
        return prefixes.map(function(prefix) {
            return prefix + _compilerName;
        }).concat([_compilerName]);
    }

    configure: {
        var result = PathProbeConfigure.configure(names, nameSuffixes, nameFilter, searchPaths,
                                                  pathSuffixes, platformSearchPaths, environmentPaths,
                                                  platformEnvironmentPaths, pathListSeparator);
        found = result.found;
        candidatePaths = result.candidatePaths;
        path = result.path;
        filePath = result.filePath;
        fileName = result.fileName;
        (nameSuffixes || [""]).forEach(function(suffix) {
            var end = _compilerName + suffix;
            if (fileName.endsWith(end))
                tcPrefix = fileName.slice(0, -end.length);
        });
    }
}
