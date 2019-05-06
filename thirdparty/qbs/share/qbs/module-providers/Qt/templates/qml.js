var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");
var Process = require("qbs.Process");
var TextFile = require("qbs.TextFile");

function scannerData(scannerFilePath, qmlFiles, qmlPath)
{
    var p;
    try {
        p = new Process();
        p.exec(scannerFilePath, ["-qmlFiles"].concat(qmlFiles).concat(["-importPath", qmlPath]),
                true);
        return JSON.parse(p.readStdOut());
    } finally {
        if (p)
            p.close();
    }
}

function getPrlRhs(line)
{
    return line.split('=')[1].trim();
}

function getLibsForPlugin(pluginData, buildVariant, targetOS, toolchain, qtLibDir)
{
    if (!pluginData.path)
        return "";
    var prlFileName = "";
    if (!targetOS.contains("windows"))
        prlFileName += "lib";
    prlFileName += pluginData.plugin;
    if (buildVariant === "debug" && targetOS.contains("windows"))
        prlFileName += "d";
    prlFileName += ".prl";
    var prlFilePath = FileInfo.joinPaths(pluginData.path, prlFileName);
    if (!File.exists(prlFilePath)) {
        console.warn("prl file for QML plugin '" + pluginData.plugin + "' not present at '"
                     + prlFilePath + "'. Linking may fail.");
        return "";
    }
    var prlFile = new TextFile(prlFilePath, TextFile.ReadOnly);
    try {
        var pluginLib;
        var otherLibs = "";
        var line;
        while (line = prlFile.readLine()) {
            if (line.startsWith("QMAKE_PRL_TARGET"))
                pluginLib = FileInfo.joinPaths(pluginData.path, getPrlRhs(line));
            if (line.startsWith("QMAKE_PRL_LIBS")) {
                var otherLibsLine = ' ' + getPrlRhs(line);
                if (toolchain.contains("msvc")) {
                    otherLibsLine = otherLibsLine.replace(/ -L/g, " /LIBPATH:");
                    otherLibsLine = otherLibsLine.replace(/-l([^ ]+)/g, "$1" + ".lib");
                }
                otherLibsLine = otherLibsLine.replace(/\$\$\[QT_INSTALL_LIBS\]/g, qtLibDir);
                otherLibs += otherLibsLine;
            }
        }
        if (!pluginLib)
            throw "Malformed prl file '" + prlFilePath + "'.";
        return pluginLib + ' ' + otherLibs;
    } finally {
        prlFile.close();
    }
}
