import qbs.FileInfo
import qbs.TextFile

QtModule {
    isPlugin: true

    property string className
    property stringList extendsModules

    enableLinking: {
        if (!base)
            return false;
        if (!isStaticLibrary)
            return false;
        if (!(Qt.plugin_support.enabledPlugins || []).contains(qtModuleName))
            return false;
        if (!extendsModules || extendsModules.length === 0)
            return true;
        for (var i = 0; i < extendsModules.length; ++i) {
            var moduleName = extendsModules[i];
            if (product.Qt[moduleName] && product.Qt[moduleName].present)
                return true;
        }
        return false;
    }

    Rule {
        condition: enableLinking
        multiplex: true
        Artifact {
            filePath: product.targetName + "_qt_plugin_import_"
                      + product.moduleProperty(product.moduleName, "qtModuleName") + ".cpp"
            fileTags: "cpp"
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            var pluginName = product.moduleProperty(product.moduleName, "qtModuleName");
            cmd.description = "Creating static import for plugin '" + pluginName + "'.";
            cmd.sourceCode = function() {
                var f = new TextFile(output.filePath, TextFile.WriteOnly);
                var className = product.moduleProperty(product.moduleName, "className");
                f.writeLine("#include <QtPlugin>\n\nQ_IMPORT_PLUGIN(" + className + ")");
                f.close();
            };
            return cmd;
        }
    }
}
