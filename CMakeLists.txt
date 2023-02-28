cmake_minimum_required(VERSION 3.25)
# import qbs
# import qbs.Process
# import qbs.TextFile
# 
# Project {
#     id: thunder
project(thunder)

set(QT_INSTALL_DIR ${CMAKE_BINARY_DIR}/Qt)
set(QT_VERSION 5.12.10)
set(VERSION 2023.1)
#setQTCREATOR_VERSION)

set(QT_MODULES
    qtbase
    qtmultimedia
    qtscript
    qtsvg
    qtimageformats
    qtgraphicaleffects
    qtquickcontrols2
    qttools
    qtxmlpatterns
    qtdeclarative
    qtgamepad
    icu
)


message("-- Installing QT")
execute_process(
    COMMAND bash ${CMAKE_SOURCE_DIR}/build/install-qt.sh -d ${QT_INSTALL_DIR} --version ${QT_VERSION} ${QT_MODULES}
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    COMMAND_ERROR_IS_FATAL ANY
)
set(CMAKE_PREFIX_PATH ${QT_INSTALL_DIR}/${QT_VERSION}/gcc_64)



# 
#     property string COMPANY_NAME: "FrostSpear"
#     property string PRODUCT_NAME: "Thunder Engine"
#     property string EDITOR_NAME: "WorldEditor"
#     property string BUILDER_NAME: "Builder"
#     property string COPYRIGHT_AUTHOR: "Evgeniy Prikazchikov"
# 
#     property string ANDROID: "android-21"
#     property string PLATFORM: {
#         var arch = qbs.architecture;
#         if(qbs.targetOS.contains("darwin") || qbs.targetOS[0] === "linux" || arch == undefined) {
#             arch = "x86_64"
#         }
#         return qbs.targetOS[0] + "/" + arch
#     }
#     property string ANDROID_STL: "c++_shared"
# 
#     property string osxVersion: "10.12"
#     property string iosVersion: "10.0"
#     property string tvosVersion: "10.0"
# 
#     property string languageVersion: "c++14"
#     property string standardLibrary: "libc++"
# 
#     property bool desktop: !qbs.targetOS.contains("android") && !qbs.targetOS.contains("ios") && !qbs.targetOS.contains("tvos")
#     property string bundle: {
#         if(qbs.targetOS.contains("darwin")) {
#             return EDITOR_NAME + ".app/Contents/MacOS/"
#         }
#         return ""
#     }
# 
#     Probe {
#         id: probe
#         property string SDK_VERSION
#         property string REVISION
#         property string LEGAL
#         property string YEAR
#         configure: {
#             YEAR = new Date().getFullYear().toString()
#             REVISION = "develop"
#             var p = new Process()
#             p.setWorkingDirectory(thunder.sourceDirectory)
#             if (p.exec("git", ["rev-parse", "HEAD"]) === 0) {
#                 REVISION = p.readStdOut().trim()
#             } else {
#                 console.error(p.readStdErr())
#             }
#             LEGAL = new TextFile(thunder.sourceDirectory + "/legal").readAll()
#             SDK_VERSION = new TextFile(thunder.sourceDirectory + "/version.cfg").readAll()
#         }
#     }
# 
#     property string COPYRIGHT_YEAR: probe.YEAR
# 
#     property string RESOURCE_ROOT: "../worldeditor/bin"
# 
#     property string PREFIX: ""
#     property string LAUNCHER_PATH: "launcher"
#     property string SDK_PATH: "sdk/" + probe.SDK_VERSION
#     property string PLATFORM_PATH: SDK_PATH + "/" + PLATFORM
#     property string BIN_PATH: PLATFORM_PATH + "/bin"
#     property string LIB_PATH: (qbs.targetOS[0] === "linux") ? PLATFORM_PATH + "/lib" : BIN_PATH
#     property string STATIC_PATH: PLATFORM_PATH + "/static"
#     property string INC_PATH: SDK_PATH + "/include"
#     property string TOOLS_PATH: SDK_PATH + "/tools"
#     property string PLUGINS_PATH: BIN_PATH + "/plugins"
# 
#     property stringList defines: {
#         var result  = [
#             "COMPANY_NAME=\"" + COMPANY_NAME + "\"",
#             "PRODUCT_NAME=\"" + PRODUCT_NAME + "\"",
#             "EDITOR_NAME=\"" + EDITOR_NAME + "\"",
#             "BUILDER_NAME=\"" + BUILDER_NAME + "\"",
#             "SDK_VERSION=\"" + probe.SDK_VERSION + "\"",
#             "COPYRIGHT_YEAR=" + COPYRIGHT_YEAR,
#             "COPYRIGHT_AUTHOR=\"" + COPYRIGHT_AUTHOR + "\"",
#             "REVISION=\"" + probe.REVISION + "\"",
#             "LEGAL=\"" + probe.LEGAL + "\""
#         ];
#         return result
#     }
# 
#     references: [
#         "thirdparty/thirdparty.qbs",
add_subdirectory(thirdparty)
#         "engine/engine.qbs",
add_subdirectory(engine)
#         "modules/modules.qbs",
#         "worldeditor/worldeditor.qbs",
#         "builder/builder.qbs",
#         "build/install.qbs",
#         "build/qbsinstall.qbs",
#         "build/tests.qbs"
#     ]
# }
# 
