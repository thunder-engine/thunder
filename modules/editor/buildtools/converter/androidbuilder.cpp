#include "androidbuilder.h"
#include "config.h"

#include <editor/projectsettings.h>
#include <editor/editorsettings.h>

#include <log.h>
#include <file.h>
#include <url.h>

#include <minizip/zip.h>

namespace {
    const char *gAndroidJava("Builder/Android/Java_Path");
    const char *gAndroidSdk("Builder/Android/SDK_Path");
    const char *gAndroidNdk("Builder/Android/NDK_Path");
    const char *gEditorPath("editor=Path");

    const char *gSdkVersion("29");
}

AndroidBuilder::AndroidBuilder() {
    setName("[AndroidBuilder]");

    EditorSettings *settings = EditorSettings::instance();

    settings->registerValue(gAndroidJava, "", gEditorPath);
    settings->registerValue(gAndroidSdk, "", gEditorPath);
    settings->registerValue(gAndroidNdk, "", gEditorPath);

    m_libs.push_back("atomic");
    m_libs.push_back("m");
    m_libs.push_back("EGL");
    m_libs.push_back("GLESv3");
    m_libs.push_back("android");
    m_libs.push_back("log");
}

bool AndroidBuilder::buildProject() {
    if(m_outdated && !m_process.isRunning()) {
        EditorSettings *settings = EditorSettings::instance();

        TString sdkPath = settings->value(gAndroidSdk).toString();
        if(sdkPath.isEmpty()) {
            aCritical() << name() << "Can't find the Android SDK by the path:" << sdkPath;
            return false;
        }

        TString ndkPath = settings->value(gAndroidNdk).toString();
        if(ndkPath.isEmpty()) {
            aCritical() << name() << "Can't find the Android NDK by the path:" << ndkPath;
            return false;
        }

        TString toolChains = ndkPath + "/toolchains/llvm/prebuilt/windows-x86_64";

        TString javaPath = EditorSettings::instance()->value(gAndroidJava).toString();
        if(javaPath.isEmpty()) {
            aCritical() << name() << "Can't find the Java by the path:" << javaPath;
            return false;
        }
        javaPath += "/bin";

        TString buildTools = sdkPath + "/build-tools/35.0.0";

        generateProject();

        ProjectSettings *mgr = ProjectSettings::instance();
        m_projectPath = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/release";
        m_artifact = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/base.apk";
        m_keystorePath = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/debug.keystore";

        File::mkPath(m_projectPath);

        m_process.setWorkingDirectory(m_project);

        static const StringList archs = {"arm64", "x86_64"};

        bool result = true;
        for(auto &it : archs) {
            result &= compileNative(toolChains, it);
        }
        if(!result) {
            return false;
        }

        compileResources(buildTools);

        if(!linkResources(buildTools, sdkPath)) {
            return false;
        }

        if(!package()) {
            return false;
        }

        if(!makeKeystore(javaPath)) {
            return false;
        }

        // Connect on last step
        connect(&m_process, _SIGNAL(finished(int)), this, _SLOT(onBuildFinished(int)));

        if(!signPackage(javaPath, buildTools)) {
            return false;
        }
    }

    return true;
}

void AndroidBuilder::generateProject() {
    NativeCodeBuilder::generateProject();

    ProjectSettings *mgr = ProjectSettings::instance();

    updateTemplate(":/templates/android/AndroidManifest.xml", mgr->platformsPath() + "/android/AndroidManifest.xml");
    updateTemplate(":/templates/android/res/xml/backup_rules.xml", mgr->platformsPath() + "/android/res/xml/backup_rules.xml");
    updateTemplate(":/templates/android/res/values/strings.xml", mgr->platformsPath() + "/android/res/values/strings.xml");

    File::mkDir(mgr->platformsPath() + "/android/res");

    StringList res = {
        "/android/res/mipmap-anydpi/ic_launcher.xml",
        "/android/res/mipmap-anydpi/ic_launcher_round.xml",
        "/android/res/drawable/ic_launcher_background.xml",
        "/android/res/drawable/ic_launcher_foreground.xml"
    };

    for(auto &it : res) {
        copyTempalte(TString(":/templates") + it, mgr->platformsPath() + it);
    }
}

bool AndroidBuilder::compileNative(const TString &tools, const TString &arch) {
    TString dir = m_projectPath + "/" + arch;
    if(!File::exists(dir) && !File::mkDir(dir)) {
        return false;
    }

    StringList args;

    TString profile = (arch == "arm64") ? "aarch64" : arch;

    args.push_back(TString("--target=") + profile + "-linux-androideabi" + gSdkVersion);
    args.push_back(TString("--sysroot=") + tools + "/sysroot");
    args.push_back("-fPIC");
    args.push_back("-g");
    args.push_back("-shared");
    args.push_back("-std=c++17");
    args.push_back("-O2");

    args.push_back("-DTHUNDER_MOBILE");
    args.push_back("-DANDROID");
    args.push_back("-D_FORTIFY_SOURCE=2");

    args.push_back("-fdata-sections");
    args.push_back("-ffunction-sections");
    args.push_back("-funwind-tables");
    args.push_back("-fstack-protector-strong");
    args.push_back("-fno-limit-debug-info");
    args.push_back("-no-canonical-prefixes");
    args.push_back("-static-libstdc++");
    args.push_back("-Qunused-arguments");
    args.push_back("-Wl,-soname,libapplication.so");
    args.push_back("-Wl,--build-id=sha1");
    args.push_back("-Wl,--no-undefined-version");
    args.push_back("-Wl,--fatal-warnings");
    args.push_back("-Wl,--no-undefined");
    args.push_back("-Wl,--strip-debug"); // To remove debug information
    args.push_back("-Wformat");
    args.push_back("-Werror=format-security");

    args.push_back("application.cpp");
    args.push_back("-o");
    args.push_back(m_projectPath + "/" + arch + "/libapplication.so");

    for(auto &it : m_incPath) {
        args.push_back(TString("-I") + it);
    }

    args.push_back(TString("-L") + ProjectSettings::instance()->sdkPath() + "/android/" + arch + "/static");

    for(auto &it : m_libs) {
        args.push_back(TString("-l") + it);
    }

    if(m_process.start(tools + "/bin/clang++" + gApplication, args) && m_process.waitForStarted()) {
        aInfo() << name() << "Compiling application for:" << arch;
        return m_process.waitForFinished();
    }

    return false;
}

bool AndroidBuilder::compileResources(const TString &tools) {
    ProjectSettings *mgr = ProjectSettings::instance();

    TString compiled = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/compiled";

    File::mkDir(compiled);

    compiled += '/';
#ifdef Q_OS_WINDOWS
    compiled.replace('/', '\\');
#endif

    for(auto &it : File::list(mgr->platformsPath() + "/android/res")) {
        if(File::isDir(it)) {
            continue;
        }
#ifdef Q_OS_WINDOWS
        it.replace('/', '\\');
#endif
        StringList args;

        args.push_back("compile");
        args.push_back(it);
        args.push_back("-o");
        args.push_back(compiled);

        if(m_process.start(tools + "/aapt2" + gApplication, args) && m_process.waitForStarted()) {
            aInfo() << name() << "Compiling resource:" << it;
            m_process.waitForFinished();
        }
    }

    return true;
}

bool AndroidBuilder::linkResources(const TString &tools, const TString &sdk) {
    ProjectSettings *mgr = ProjectSettings::instance();

    StringList args;

    args.push_back("link");
    args.push_back("-o");
    args.push_back(m_artifact);
    args.push_back("-I");
    args.push_back(sdk + "/platforms/android-" + gSdkVersion + "/android.jar");
    args.push_back("--manifest");
    args.push_back(mgr->platformsPath() + "/android/AndroidManifest.xml");
    args.push_back("--min-sdk-version");
    args.push_back(gSdkVersion);
    args.push_back("--target-sdk-version");
    args.push_back("30");
    args.push_back("--version-code");
    args.push_back("1");
    args.push_back("--version-name");
    args.push_back(mgr->projectVersion());

    for(auto &it : File::list(mgr->cachePath() + "/" + mgr->currentPlatformName() + "/compiled")) {
        args.push_back(it);
    }

    args.push_back("--auto-add-overlay");

    if(m_process.start(tools + "/aapt2" + gApplication, args) && m_process.waitForStarted()) {
        aInfo() << name() << "Linking Resources.";
        return m_process.waitForFinished();
    }

    return false;
}

bool AndroidBuilder::makeKeystore(const TString &java) {
    if(File::exists(m_keystorePath)) {
        return true;
    }

    StringList args;

    args.push_back("-genkey");
    args.push_back("-v");
    args.push_back("-keystore");
    args.push_back(m_keystorePath);
    args.push_back("-alias");
    args.push_back("androiddebugkey");
    args.push_back("-keyalg");
    args.push_back("RSA");
    args.push_back("-keysize");
    args.push_back("2048");
    args.push_back("-validity");
    args.push_back("10000");
    args.push_back("-storepass");
    args.push_back("android");
    args.push_back("-keypass");
    args.push_back("android");
    args.push_back("-dname");
    args.push_back("\"CN=Android Debug,O=Android,C=US\"");

    if(m_process.start(java + "/keytool" + gApplication, args) && m_process.waitForStarted()) {
        aInfo() << name() << "Generate temporary keystore for signing.";
        return m_process.waitForFinished();
    }

    return false;
}

bool AndroidBuilder::signPackage(const TString &java, const TString &tools) {
    ProjectSettings *mgr = ProjectSettings::instance();

    StringList args;

    TString out = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/signed.apk";

    args.push_back("-Xmx1024M");
    args.push_back("-Xss1m");
    args.push_back("-jar");
    args.push_back(tools + "/lib/apksigner.jar");
    args.push_back("sign");
    args.push_back("--ks");
    args.push_back(m_keystorePath);
    args.push_back("--ks-pass");
    args.push_back("pass:android");
    args.push_back("--out");
    args.push_back(out);
    args.push_back(m_artifact);

    mgr->setArtifacts({ out });

    if(m_process.start(java + "/java" + gApplication, args) && m_process.waitForStarted()) {
        aInfo() << name() << "Package signing.";
        return m_process.waitForFinished();
    }

    return false;
}

bool AndroidBuilder::package() {
    zipFile zf = zipOpen(m_artifact.data(), APPEND_STATUS_ADDINZIP);
    if(!zf) {
        aError() << "Can't open package.";
        return false;
    }

    aInfo() << name() << "Assets packaging.";

    zip_fileinfo zi = {0};

    StringList list(File::list(ProjectSettings::instance()->importPath()));
    for(auto &it : list) {
        Url info(it);

        File inFile(it);
        if(!inFile.open(File::ReadOnly)) {
            zipClose(zf, nullptr);
            aError() << "Can't open input file.";
            return false;
        }

        zipOpenNewFileInZip(zf, (TString("assets/") + info.name()).data(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

        ByteArray data(inFile.readAll());
        inFile.close();

        zipWriteInFileInZip(zf, data.data(), data.size());
        zipCloseFileInZip(zf);
    }

    aInfo() << name() << "Native code packaging.";

    for(auto &it : File::list(m_projectPath)) {
        if(File::isDir(it)) {
            continue;
        }

        File inFile(it);
        if(!inFile.open(File::ReadOnly)) {
            zipClose(zf, nullptr);
            aError() << "Can't open input file.";
            return false;
        }

        TString dir = Url(it).dir().split('/').back();

        zipOpenNewFileInZip(zf, (TString("lib/") + ((dir == "arm64") ? "arm64-v8a" : dir) + "/libapplication.so").data(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_NO_COMPRESSION);

        ByteArray data(inFile.readAll());
        inFile.close();

        zipWriteInFileInZip(zf, data.data(), data.size());
        zipCloseFileInZip(zf);
    }

    zipClose(zf, nullptr);

    return true;
}
