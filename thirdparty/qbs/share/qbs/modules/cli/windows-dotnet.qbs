import qbs.Utilities

CLIModule {
    condition: qbs.toolchain && qbs.toolchain.contains("dotnet")

    debugInfoSuffix: ".pdb"
    csharpCompilerName: "csc"
    vbCompilerName: "vbc"
    fsharpCompilerName: "fsc"

    Probe {
        id: dotnetProbe

        property string toolchainInstallPath // Output

        configure: {
            // https://msdn.microsoft.com/en-us/library/hh925568.aspx
            var keys = [
                "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP",
                "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\NET Framework Setup\\NDP"
            ];
            for (var i in keys) {
                var key = keys[i] + "\\v4\\Full";
                var value = Utilities.getNativeSetting(key, "InstallPath");
                if (value) {
                    toolchainInstallPath = value;
                    found = true;
                    break;
                }
            }
        }
    }

    toolchainInstallPath: dotnetProbe.toolchainInstallPath
}
