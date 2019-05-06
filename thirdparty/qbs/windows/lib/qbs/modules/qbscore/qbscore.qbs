import qbs
import qbs.FileInfo

Module {
    cpp.defines: []
    cpp.includePaths: ["/include/qbs", ]
    Group {
        filesAreTargets: true
        fileTags: ["hpp"]
        files: [
            "../../../../include/qbs/tools/architectures.h",
            "../../../../include/qbs/tools/buildoptions.h",
            "../../../../include/qbs/tools/cleanoptions.h",
            "../../../../include/qbs/tools/codelocation.h",
            "../../../../include/qbs/tools/commandechomode.h",
            "../../../../include/qbs/tools/error.h",
            "../../../../include/qbs/tools/generateoptions.h",
            "../../../../include/qbs/tools/installoptions.h",
            "../../../../include/qbs/tools/joblimits.h",
            "../../../../include/qbs/tools/preferences.h",
            "../../../../include/qbs/tools/processresult.h",
            "../../../../include/qbs/tools/profile.h",
            "../../../../include/qbs/tools/projectgeneratormanager.h",
            "../../../../include/qbs/tools/qbs_export.h",
            "../../../../include/qbs/tools/settings.h",
            "../../../../include/qbs/tools/settingsmodel.h",
            "../../../../include/qbs/tools/settingsrepresentation.h",
            "../../../../include/qbs/tools/setupprojectparameters.h",
            "../../../../include/qbs/tools/toolchains.h",
            "../../../../include/qbs/tools/version.h",
            "../../../../include/qbs/buildgraph/forward_decls.h",
            "../../../../include/qbs/generators/generator.h",
            "../../../../include/qbs/generators/generatordata.h",
            "../../../../include/qbs/language/forward_decls.h",
            "../../../../include/qbs/logging/ilogsink.h",
            "../../../../include/qbs/qbs.h",
            "../../../../include/qbs/api/jobs.h",
            "../../../../include/qbs/api/languageinfo.h",
            "../../../../include/qbs/api/project.h",
            "../../../../include/qbs/api/projectdata.h",
            "../../../../include/qbs/api/rulecommand.h",
            "../../../../include/qbs/api/runenvironment.h",
            "../../../../include/qbs/api/transformerdata.h",
        ]
    }
    Group {
        filesAreTargets: true
        fileTags: ["unknown-file-tag"]
        files: [
            "../../../../include/qbs/use_installed_corelib.pri",
            "../../../../include/qbs/qbs_version.pri",
        ]
    }
    Group {
        filesAreTargets: true
        fileTags: ["dynamiclibrary"]
        files: [
            "../../../../bin/qbscore.dll",
        ]
    }
    Group {
        filesAreTargets: true
        fileTags: ["dynamiclibrary_import"]
        files: [
            "../../../qbscore.lib",
        ]
    }
    Depends {
        name: "cpp"
    }
    Depends {
        name: "Qt";
        submodules: ["core"]
    }
    Properties {
        condition: product.hasExporter
    }
    Depends {
        name: "cpp"
    }
}
