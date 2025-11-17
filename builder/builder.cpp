#include "builder.h"

#include <log.h>
#include <editor/projectsettings.h>
#include <editor/pluginmanager.h>
#include <editor/editorsettings.h>
#include <editor/assetmanager.h>
#include <editor/nativecodebuilder.h>

#include <minizip/zip.h>

#include <QCoreApplication>

Builder::Builder() {
    connect(AssetManager::instance(), &AssetManager::importFinished, this, &Builder::onImportFinished, Qt::QueuedConnection);
    connect(AssetManager::instance(), &AssetManager::buildSuccessful, this, &Builder::onBuildSuccessful, Qt::QueuedConnection);
}

void Builder::setPlatform(const TString &platform) {
    ProjectSettings *project = ProjectSettings::instance();
    EditorSettings::instance()->loadSettings();
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

        AssetManager::instance()->rescan();
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

    StringList list(File::list(ProjectSettings::instance()->importPath()));
    for(auto &it : list) {
        if(File::isFile(it)) {
            Url info(it);

            TString origin = AssetManager::instance()->uuidToPath(info.baseName());
            aInfo() << "\tCoping:" << origin.data();

            File inFile(it);
            if(!inFile.open(File::ReadOnly)) {
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
    ProjectSettings *project = ProjectSettings::instance();

    NativeCodeBuilder *builder = project->currentBuilder();

    if(builder) {
        if(builder->packagingMode() == NativeCodeBuilder::Before) {
            package(project->cachePath() + "/" + project->currentPlatformName());
        }

        builder->buildProject();
    }
}

void Builder::onBuildSuccessful() {
    ProjectSettings *project = ProjectSettings::instance();
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
            AssetManager::instance()->rescan();

            return;
        }
    }

    QCoreApplication::exit(0);
}
