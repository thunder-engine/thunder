# import qbs
# 
# Project {
#     id: codeeditor
#     property stringList srcFiles: [
#         "*.cpp",
#         "editor/*.cpp",
#         "*.qrc",
#         "*.h",
#         "editor/*.h",
#         "editor/*.ui",
#     ]
# 
#     property stringList incPaths: [
#         "editor",
#         "../../../engine/includes",
#         "../../../engine/includes/resources",
#         "../../../engine/includes/editor",
#         "../../../thirdparty/next/inc",
#         "../../../thirdparty/next/inc/math",
#         "../../../thirdparty/next/inc/core",
#         "../../../thirdparty/syntaxhighlighting/src"
#     ]
# 
#     DynamicLibrary {
#         name: "codeeditor"
#         condition: codeeditor.desktop
#         files: codeeditor.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         Depends { name: "next-editor" }
#         Depends { name: "engine-editor" }
#         Depends { name: "syntaxhighlighting" }
#         Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
#         bundle.isBundle: false
# 
#         cpp.defines: ["SHARED_DEFINE"]
#         cpp.includePaths: codeeditor.incPaths
#         cpp.cxxLanguageVersion: codeeditor.languageVersion
#         cpp.cxxStandardLibrary: codeeditor.standardLibrary
#         cpp.minimumMacosVersion: codeeditor.osxVersion
# 
#         Properties {
#             condition: qbs.targetOS.contains("darwin")
#             cpp.sonamePrefix: "@executable_path"
#         }
# 
#         Group {
#             name: "Install Plugin"
#             fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
#             qbs.install: true
#             qbs.installDir: codeeditor.PLUGINS_PATH
#             qbs.installPrefix: codeeditor.PREFIX
#         }
#     }
# }
