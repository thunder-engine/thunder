Module {
    // Set by user.
    property varList pluginsByType

    // Set by Qt modules.
    property stringList pluginTypes

    // Set by setup-qt.
    readonly property var allPluginsByType: @allPluginsByType@
    readonly property stringList nonEssentialPlugins: @nonEssentialPlugins@

    // Derived.
    readonly property var defaultPluginsByType: {
        var map = {};
        for (var i = 0; i < (pluginTypes || []).length; ++i) {
            var pType = pluginTypes[i];
            map[pType] = (allPluginsByType[pType] || []).filter(function(p) {
                return !nonEssentialPlugins.contains(p); });
        }
        return map;
    }
    readonly property var effectivePluginsByType: {
        var ppt = pluginsByType || [];
        var eppt = {};
        for (var i = 0; i < ppt.length; ++i) {
            var listEntry = ppt[i];
            for (var pluginType in listEntry) {
                var newValue = listEntry[pluginType];
                if (!newValue)
                    newValue = [];
                else if (typeof newValue == "string")
                    newValue = [newValue];
                if (!Array.isArray(newValue))
                    throw "Invalid value '" + newValue + "' in Qt.plugin_support.pluginsByType";
                eppt[pluginType] = (eppt[pluginType] || []).uniqueConcat(newValue);
            }
        }
        var dppt = defaultPluginsByType;
        for (var pluginType in dppt) {
            if (!eppt[pluginType])
                eppt[pluginType] = dppt[pluginType];
        }
        return eppt;
    }
    readonly property stringList enabledPlugins: {
        var list = [];
        var eppt = effectivePluginsByType;
        for (var t in eppt)
            Array.prototype.push.apply(list, eppt[t]);
        return list;
    }

    validate: {
        var ppt = pluginsByType;
        if (!ppt)
            return;
        var appt = allPluginsByType;
        for (var i = 0; i < ppt.length; ++i) {
            for (var pluginType in ppt[i]) {
                var requestedPlugins = ppt[i][pluginType];
                if (!requestedPlugins)
                    continue;
                var availablePlugins = appt[pluginType] || [];
                if (typeof requestedPlugins === "string")
                    requestedPlugins = [requestedPlugins];
                for (var j = 0; j < requestedPlugins.length; ++j) {
                    if (!availablePlugins.contains(requestedPlugins[j])) {
                        throw "Plugin '" + requestedPlugins[j] +  "' of type '" + pluginType
                                + "' was requested, but is not available.";
                    }
                }
            }
        }
    }
}
