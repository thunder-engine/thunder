import qbs.File
import qbs.Probes

CLIModule {
    condition: qbs.toolchain && qbs.toolchain.contains("mono")

    debugInfoSuffix: ".mdb"
    csharpCompilerName: "mcs"
    vbCompilerName: "vbnc"
    fsharpCompilerName: "fsharpc"

    Probes.PathProbe {
        id: monoProbe
        names: ["mono"]
        platformSearchPaths: {
            var paths = [];
            if (qbs.hostOS.contains("macos"))
                paths.push("/Library/Frameworks/Mono.framework/Commands");
            if (qbs.hostOS.contains("unix"))
                paths.push("/usr/bin");
            return paths;
        }
    }

    toolchainInstallPath: monoProbe.path
}
