#include "builder.h"

#include <log.h>
#include <url.h>
#include <file.h>
#include <editor/projectsettings.h>
#include <editor/pluginmanager.h>
#include <editor/editorsettings.h>
#include <editor/assetmanager.h>
#include <editor/nativecodebuilder.h>

#include <compat/zip.h>

#include <QCoreApplication>

Builder::Builder() {
    connect(Editor::assets(), &AssetManager::importFinished, this, &Builder::onImportFinished, Qt::QueuedConnection);
    connect(Editor::assets(), &AssetManager::buildSuccessful, this, &Builder::onBuildSuccessful, Qt::QueuedConnection);
}

void Builder::setPlatform(const TString &platform) {
    ProjectSettings *project = Editor::project();
    Editor::settings()->loadSettings();
    if(platform.isEmpty()) {
        for(const TString &it : project->platforms()) {
            m_platformsToBuild.push(it);
        }
    } else {
        m_platformsToBuild.push(platform);
    }

    if(!m_platformsToBuild.empty()) {
        project->setCurrentPlatform(m_platformsToBuild.top());
        m_platformsToBuild.pop();

        NativeCodeBuilder *builder = project->currentBuilder();
        if(builder) {
            builder->convertFile(nullptr);
        }

        Editor::assets()->rescan();
    }
}

bool Builder::package(const TString &target) {
    TString pak = target + "/base.pak";

    aInfo() << "Packaging Assets to:" << pak << target;

    zipFile zf = zipOpen(pak.data(), 0);
    if(!zf) {
        aError() << "Can't open package.";
        return false;
    }

    StringList list(File::list(Editor::project()->importPath()));
    for(auto &it : list) {
        if(File::isFile(it)) {
            Url info(it);

            TString origin = Editor::assets()->uuidToPath(info.baseName());
            aInfo() << "\tCoping:" << origin.data();

            File inFile(it);
            if(!inFile.open(File::Read)) {
                zipClose(zf, nullptr);
                aError() << "Can't open input file.";
                return false;
            }

            zip_fileinfo zi = {0};
            zipOpenNewFileInZip(zf, info.name().data(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_NO_COMPRESSION);

            ByteArray data(inFile.readAll());
            inFile.close();

            zipWriteInFileInZip(zf, data.data(), data.size());
            zipCloseFileInZip(zf);
        }
    }

    zipClose(zf, nullptr);

    aInfo() << "Packaging Done.";
    return true;
}

void Builder::onImportFinished() {
    ProjectSettings *project = Editor::project();

    NativeCodeBuilder *builder = project->currentBuilder();

    if(builder) {
        if(builder->packagingMode() == NativeCodeBuilder::Before) {
            package(project->cachePath() + "/" + project->currentPlatformName());
        }

        builder->buildProject();
    }
}

void Builder::onBuildSuccessful() {
    ProjectSettings *project = Editor::project();
    TString targetPath = project->targetPath() + "/" + project->currentPlatformName();

    if(!File::exists(targetPath) && !File::mkPath(targetPath)) {
        aDebug() << "Unable to create build directory at:" << targetPath;
    }

    // Clean install dir
    for(auto &it : File::list(targetPath)) {
        File::remove(it);
    }

    bool result = true;
    for(const TString &it : project->artifacts()) {
        result &= File::copy(it, targetPath + "/" + Url(it).name());
    }

    if(result) {
        aInfo() << "New build copied to:" << targetPath;

        // Package after
        NativeCodeBuilder *builder = project->currentBuilder();
        if(builder && builder->packagingMode() == NativeCodeBuilder::After) {
            // Package right to install dir
            package(targetPath);
        }

        if(!m_platformsToBuild.empty()) {
            project->setCurrentPlatform(m_platformsToBuild.top());
            m_platformsToBuild.pop();
            Editor::assets()->rescan();

            return;
        }
    }

    QCoreApplication::exit(0);
}
