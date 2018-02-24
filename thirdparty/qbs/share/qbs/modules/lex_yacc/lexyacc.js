var FileInfo = require("qbs.FileInfo");

function outputFilePath(product, input, posixFileName, forYacc)
{
    var outDir = product.moduleProperty("lex_yacc", "outputDir");
    var fileName;
    if (product.moduleProperty("lex_yacc", "uniqueSymbolPrefix")) {
        fileName = input.baseName;
        if (forYacc)
            fileName += posixFileName.slice(1);
        else
            fileName += posixFileName;
    } else {
        fileName = posixFileName;
    }
    return FileInfo.joinPaths(outDir, fileName);
}
